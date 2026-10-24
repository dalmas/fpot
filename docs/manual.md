# Manual de dados de entrada do FLP

Este documento resume o formato do arquivo de entrada suportado pelo parser implementado em `src/inpanrd.c`. O escopo aqui descrito é restrito ao que realmente é lido e interpretado pelo programa; blocos não implementados ou apenas reconhecidos para avançar o arquivo são destacados como tal.

## Capacidade

| Descrição | Número de elementos |
|-----------|---------------------|
| Barras    | 1000                |
| Linhas    | 1000                |
| Geração   | 1000                |

## Códigos de execução suportados

| Código | Descrição | Status no parser |
|--------|-----------|------------------|
| TITU   | Leitura do título do caso. | Implementado |
| DCTE   | Leitura/modificação de constantes do problema. | Implementado |
| DBAR   | Leitura dos dados de barra CA. | Implementado |
| DLIN   | Leitura dos dados de circuito CA. | Implementado |
| DGER   | Leitura dos limites de geração ativa. | Implementado |
| DGLT   | Limites de tensão por grupo. | Reconhecido, mas não processado |
| DGBT   | Dados de grupo base de tensão. | Reconhecido, mas não processado |
| FIM    | Término de execução. | Implementado |

## Observações gerais sobre a leitura

- O arquivo de entrada é processado em linhas de texto com comprimento máximo de 80 caracteres (`LINE_LENGHT = 80`).
- Linhas iniciadas por `(` são ignoradas como comentários.
- O parser usa a primeira palavra da linha para identificar o código de execução (`sscanf(line, "%s", excode);`).
- O bloco `DCTE` termina ao encontrar `9999` na primeira coluna da linha.
- Os blocos de barras e circuitos terminam ao encontrar `9999` na primeira coluna da linha.
- `DGLT` e `DGBT` são aceitos apenas para consumir registros até o terminador `99` e não são convertidos em estruturas internas.

## TITU

### Objetivo

Ler o título do caso em estudo.

### Conjunto de dados

1. Cartão com o código `TITU` nas colunas 1-4.
2. Cartão seguinte contendo a identificação do caso.

### Formato

| Campo | Colunas | Descrição |
|-------|---------|-----------|
| Título do caso | 1-80 | Texto livre usado como identificação do estudo. |

O programa lê a linha imediatamente após `TITU` e a copia para a variável global `titu`.

## DCTE

### Objetivo

Alterar ou inicializar os valores das constantes internas do programa.

### Conjunto de dados

1. Código `DCTE`.
2. Um ou mais pares `mnemonico valor`.
3. Registro `9999` como fim do bloco.

### Constantes reconhecidas

| Mnemonico | Significado | Valor inicial no código |
|-----------|-------------|------------------------|
| `TEPA` | Tolerância para erro de potência ativa. | `1 / base` |
| `TEPR` | Tolerância para erro de potência reativa. | `1 / base` |
| `BASE` | Base de potência do sistema CA. | `100.0` |
| `ACIT` | Número máximo de iterações do fluxo CA. | `30` |

### Formato dos dados

A leitura é feita em blocos de 12 posições, em sequência, interpretando `mnemônico` e `valor` em cada trecho. A estrutura esperada é do tipo:

`MNEMÔNICO VALOR MNEMÔNICO VALOR ...`

No código, o parser faz uma leitura sequencial com `sscanf(&line[i], "%s %lf", mnm, &cte)` e incrementa `i += 12` após cada par.

### Observações

- O programa somente reconhece os mnemonicos `TEPA`, `TEPR`, `BASE` e `ACIT`.
- Qualquer outro mnemônico em `DCTE` é ignorado.
- O valor da base de potência também afeta o cálculo de grandezas escalares em blocos posteriores, como `dbar[nbar].qn`, `qm`, `sh` e `dlin[nlin].b`.

## DBAR

### Objetivo

Definir as barras do sistema CA.

### Conjunto de dados

1. Código `DBAR`.
2. Dados de uma ou mais barras.
3. Registro `9999` como fim do bloco.

### Formato da linha

A linha é interpretada em posições fixas. O parser exige o campo de número da barra em 1-4; se estiver em branco, a leitura falha com mensagem `Bar number field is mandatory.`

| Campo | Colunas | Descrição |
|-------|---------|-----------|
| `id` | 1-4 | Número da barra. |
| `operação` | 6 | Operação da barra. Aceita espaço, `A` ou `0`; `E`/`1`/`M`/`2` geram erro “not implemented”. |
| `tipo` | 8 | Tipo da barra: `0` = PQ, `1` = PV, `2` = slack, `3` = PQ com comportamento especial. |
| `base` | 9 | Letra da base da barra (`0` se vazio). |
| `nome` | 10-21 | Nome da barra (12 caracteres). |
| `group` | 22 | Grupo da barra (`0` se vazio). |
| `voltage` | 23-26 | Tensão nominal em kV, lida em `0.001 * atoi(...)`. |
| `phase` | 27-30 | Ângulo em graus, convertido para radianos com `M_PI * phase / 180`. |
| `pg` | 31-35 | Geração ativa, em MW. |
| `qg` | 36-40 | Geração reativa, em Mvar. |
| `qn` | 41-45 | Limite inferior de potência reativa. Se vazio, assume `0.` ou `-9999.` para barras tipo 2. |
| `qm` | 46-50 | Limite superior de potência reativa. Se vazio, assume `0.` ou `9999.` para barras tipo 2. |
| `bc` | 51-55 | Código da barra de conexão; se vazio, usa o próprio número da barra. |
| `pl` | 56-60 | Carga ativa. |
| `ql` | 61-65 | Carga reativa. |
| `sh` | 66-70 | Carga de shunt. |
| `area` | 71-72 | Área da barra. |
| `cf` | 73-76 | Fator de correção, lido em `0.001 * atoi(...)`. |

### Tipos de barra suportados

| Valor em coluna 8 | Tipo interno | Significado |
|------------------|-------------|-------------|
| `0` | `0` | PQ |
| `1` | `1` | PV |
| `2` | `2` | Slack |
| `3` | `3` | PQ com outra semântica (aceita em leitura) |

### Observações

- `npv`, `npq` e `nbar` são reinicializados no começo do bloco.
- O programa aceita apenas operação vazia, `A` ou `0`; qualquer operação de exclusão ou modificação (`E`, `1`, `M`, `2`) gera erro.
- `qn` e `qm` são convertidos para p.u. dividindo por `base`.
- `sh` também é convertido para p.u. dividindo por `base`.

## DLIN

### Objetivo

Definir os circuitos do sistema CA.

### Conjunto de dados

1. Código `DLIN`.
2. Um ou mais registros de circuito.
3. Registro `9999` como fim do bloco.

### Formato da linha

| Campo | Colunas | Descrição |
|-------|---------|-----------|
| `from` | 1-4 | Barra de origem. |
| `operação` | 6 | Operação do circuito. Aceita espaço, `A` ou `0`; `E`/`1`/`M`/`2` geram erro. |
| `to` | 9-12 | Barra de destino. |
| `r` | 17-22 | Resistência em % (`0.01 * atof(...)` se informado). |
| `x` | 23-28 | Reatância em % (`0.01 * atof(...)` se informado). |
| `b` | 29-34 | Susceptância, dividida por `base` quando informada (`atof(data) / base`). |
| `tap` | 35-39 | Tap do transformador. |
| `tmn` | 40-44 | Limite mínimo do tap. |
| `tmx` | 45-49 | Limite máximo do tap. |

### Observações

- Caso algum campo de resistência, reatância ou susceptância esteja em branco, o valor lido é `0.`.
- O programa também interpreta `tap`, `tmn` e `tmx` como valores em ponto flutuante, sem validação adicional.
- Há uma repetição no código para `tmn`, onde o mesmo trecho do arquivo é lido duas vezes; o comportamento final é o mesmo.

## DGER

### Objetivo

Ler os limites de geração ativa de cada gerador.

### Conjunto de dados

1. Código `DGER`.
2. Um ou mais registros de geração.
3. Registro `9999` como fim do bloco.

### Formato da linha

| Campo | Colunas | Descrição |
|-------|---------|-----------|
| `id` | 1-4 | Número do gerador (identificador da barra). |
| `pmn` | 8-12 | Limite mínimo de geração ativa. Se vazio, assume `0.`; divide por `base`. |
| `pmx` | 14-18 | Limite máximo de geração ativa. Se vazio, assume `0.`; divide por `base`. |

O parser armazena esses valores na estrutura `dger_t` e contabiliza `nger`.

## DGLT

### Objetivo

Bloco de limites de tensão por grupo, conforme a especificação do FLUPOT.

### Status no código

O parser reconhece o código `DGLT`, mas não lê nem armazena seus campos em estruturas. Ele apenas percorre as linhas até encontrar o terminador `99`:

```c
if (strncmp(line, "99", 2) == 0) {
  break;
}
```

Portanto, o bloco é aceito para permitir a continuidade da leitura do arquivo, mas não há implementação completa para os dados de `DGLT`.

## DGBT

### Objetivo

Bloco de grupos base de tensão.

### Status no código

Assim como em `DGLT`, o parser reconhece `DGBT`, mas não interpreta os campos do registro. Ele apenas ignora linhas até encontrar o terminador `99`.

Portanto, neste programa, `DGBT` não é convertido em dados internos nem usado em cálculos.

## FIM

### Objetivo

Encerrar a leitura do arquivo de entrada.

Quando o código `FIM` é identificado, a função `input_data()` retorna `0`, indicando leitura concluída com sucesso.

## Exemplo mínimo de arquivo

```text
TITU
Sistema exemplo
DCTE
BASE   100. TEPA   0.01 TEPR   0.01 ACIT   30
9999
DBAR
0001   2 ...
...
9999
DLIN
0001    0004      5.76       1.00
...
9999
DGER
0001    0.00 247.5
...
9999
FIM
```

Este exemplo ilustra a estrutura geral dos blocos; os dados reais são processados por coluna e não por nomes de campos em texto livre.
