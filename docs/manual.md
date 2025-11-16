# Capacidade

|Descrição | Número de elementos |
|----------|---------------------|
| Barras   | 1000                |
| Linhas   | 1000                |
| Geração  | 1000                |

<!--------------------------------------------------------------------------------------->

# Códigos de execução

|Código | Descrição                                   |
|-------|---------------------------------------------|
|TITU   | Leitura do título.                          |
|DCTE   | Leitura/modificação de dados de constantes. |
|DBAR   | Leitura dos dados de barra CA.              |
|DLIN   | Leitura dos dados de circuito CA.           |
|DGER   | Leitura dos dados de barra de geração.      |
|DGBT   | Leitura dos dados de grupo base de tensão.  |
|FIM    | Término de execução.                        | 

<!--------------------------------------------------------------------------------------->

## TITU

Leitura do título do caso em estudo. 

<!--------------------------------------------------------------------------------------->

## DCTE Constantes

| Campo | Descrição                                                        | Default   |
|-------|------------------------------------------------------------------|-----------|
| TEPA  | Tolerância de convergência do erro de potência ativa na barra.   | 1.0 MW    |
| TEPR  | Tolerância de convergência do erro de potência reativa na barra. | 1 Mvar    |
| BASE  | Base de potência para o sistema CA.                              | 100.0 MVA |
| ACIT  | Número máximo de iterações na solução do fluxo de potência CA.   | 30        |

### Conjunto de dados

1. Código DCTE
1. Constantes
1. 9999 nas colunas 1-4 indicando fim de dados

### Formato dos Dados

| Campo     | Colunas                                            | Descrição                                                            |
|-----------|----------------------------------------------------|----------------------------------------------------------------------|
| Mnmônico  | 01-04<br>13-16<br>25-28<br>37-40<br>49-52<br>61-64 | Mnemônicos correspondentes às constantes a serem modificadas.        |
| Constante | 06-11<br>18-23<br>30-35<br>42-47<br>54-59<br>66-71 | Constantes associadas aos mnemônicos definido no campo do Mnemônico. |

<!--------------------------------------------------------------------------------------->

## DBAR Barras CA

Leitura dos dados de barra CA. 

### Conjunto de dados

1. Código DBAR
1. Dados da barra
1. 9999 nas colunas 1-4 indicando fim de dados

### Formato dos Dados

| Campo | Colunas | Descrição | Default | Observação |
|-------|---------|-----------|---------|------------|
|Número |01-05    |Número de identificação da barra CA | | |
|Operação| 06-06 | A ou 0 - adição de dados de barra.<br>E ou 1 - eliminação de dados da barra.<br>M ou 2 - modificação de dados da barra. | A | |

<!--------------------------------------------------------------------------------------->

## DLIN 

Leitura dos dados de circuito CA.

### Conjunto de dados

1. Código DLIN

### Formato dos Dados

| Campo | Colunas | Descrição | Default | Observação |
|-------|---------|-----------|---------|------------|

<!--------------------------------------------------------------------------------------->

## DGER

Leitura dos dados de barra de geração.

### Conjunto de dados

1. Código DGER

### Formato dos Dados

| Campo | Colunas | Descrição | Default | Observação |
|-------|---------|-----------|---------|------------|

<!--------------------------------------------------------------------------------------->

## DGBT

Leitura dos dados de grupo base de tensão.

### Conjunto de dados

1. Código DGBT

### Formato dos Dados

| Campo | Colunas | Descrição | Default | Observação |
|-------|---------|-----------|---------|------------|

<!--------------------------------------------------------------------------------------->

## FIM

Término de execução.
