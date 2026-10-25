# Input Data Manual

This document summarizes the input file format supported by the parser implemented in `src/inpanrd.c`. It describes only what the program actually reads and interprets; unsupported blocks and blocks that are recognized only to advance through the file are identified as such.

## Capacity

| Description | Number of elements |
|-------------|--------------------|
| Buses       | 1000               |
| Lines       | 1000               |
| Generators  | 1000               |

## Supported execution codes

| Code | Description | Parser status |
|------|-------------|---------------|
| TITU | Read the case title. | Implemented |
| DCTE | Read or modify problem constants. | Implemented |
| DBAR | Read AC bus data. | Implemented |
| DLIN | Read AC circuit data. | Implemented |
| DGER | Read active generation limits. | Implemented |
| DGLT | Voltage limits by group. | Recognized, but not processed |
| DGBT | Base-voltage group data. | Recognized, but not processed |
| FIM  | End processing. | Implemented |

## General parsing notes

- The input file is processed as text lines with a maximum length of 80 characters (`LINE_LENGHT = 80`).
- Lines beginning with `(` are ignored as comments.
- The parser uses the first word on each line to identify the execution code (`sscanf(line, "%s", excode);`).
- The `DCTE` block ends when `9999` is found in the first column.
- Bus and circuit blocks end when `9999` is found in the first column.
- `DGLT` and `DGBT` are accepted only to consume records up to the `99` terminator; their data is not stored in internal structures.

## TITU

### Purpose

Read the title of the case being studied.

### Records

1. A record with the `TITU` code in columns 1-4.
2. The following record contains the case identifier.

### Format

| Field | Columns | Description |
|-------|---------|-------------|
| Case title | 1-80 | Free text used to identify the study. |

The program reads the line immediately after `TITU` and copies it to the global variable `titu`.

## DCTE

### Purpose

Set or initialize the program's internal constants.

### Records

1. The `DCTE` code.
2. One or more `mnemonic value` pairs.
3. The `9999` record marks the end of the block.

### Recognized constants

| Mnemonic | Meaning | Initial value in code |
|----------|---------|-----------------------|
| `TEPA` | Active power error tolerance. | `1 / base` |
| `TEPR` | Reactive power error tolerance. | `1 / base` |
| `BASE` | AC system power base. | `100.0` |
| `ACIT` | Maximum number of AC power-flow iterations. | `30` |

### Data format

The data is read sequentially in 12-character segments, interpreting a `mnemonic` and `value` in each segment. The expected layout is:

`MNEMONIC VALUE MNEMONIC VALUE ...`

The parser reads each pair sequentially with `sscanf(&line[i], "%s %lf", mnm, &cte)` and increments `i += 12` after each pair.

### Notes

- The program recognizes only the `TEPA`, `TEPR`, `BASE`, and `ACIT` mnemonics.
- Any other mnemonic in `DCTE` is ignored.
- The power base also affects the calculation of scalar values in later blocks, including `dbar[nbar].qn`, `qm`, `sh`, and `dlin[nlin].b`.

## DBAR

### Purpose

Define the AC system buses.

### Records

1. The `DBAR` code.
2. One or more bus records.
3. The `9999` record marks the end of the block.

### Line format

The line is interpreted using fixed column positions. The parser requires the bus number in columns 1-4; if this field is blank, reading fails with the message `Bar number field is mandatory.`

| Field | Columns | Description |
|-------|---------|-------------|
| `id` | 1-4 | Bus number. |
| `operation` | 6 | Bus operation. A blank, `A`, or `0` is accepted; `E`/`1`/`M`/`2` produce a “not implemented” error. |
| `type` | 8 | Bus type: `0` = PQ, `1` = PV, `2` = slack, `3` = PQ with special behavior. |
| `base` | 9 | Bus base letter (`0` if blank). |
| `name` | 10-21 | Bus name (12 characters). |
| `group` | 22 | Bus group (`0` if blank). |
| `voltage` | 23-26 | Nominal voltage in kV, read as `0.001 * atoi(...)`. |
| `phase` | 27-30 | Angle in degrees, converted to radians with `M_PI * phase / 180`. |
| `pg` | 31-35 | Active generation, in MW. |
| `qg` | 36-40 | Reactive generation, in Mvar. |
| `qn` | 41-45 | Lower reactive power limit. If blank, defaults to `0.` or `-9999.` for type 2 buses. |
| `qm` | 46-50 | Upper reactive power limit. If blank, defaults to `0.` or `9999.` for type 2 buses. |
| `bc` | 51-55 | Connection bus code; if blank, the bus number itself is used. |
| `pl` | 56-60 | Active load. |
| `ql` | 61-65 | Reactive load. |
| `sh` | 66-70 | Shunt load. |
| `area` | 71-72 | Bus area. |
| `cf` | 73-76 | Correction factor, read as `0.001 * atoi(...)`. |

### Supported bus types

| Value in column 8 | Internal type | Meaning |
|-------------------|---------------|---------|
| `0` | `0` | PQ |
| `1` | `1` | PV |
| `2` | `2` | Slack |
| `3` | `3` | PQ with different semantics (accepted on input) |

### Notes

- `npv`, `npq`, and `nbar` are reset at the beginning of the block.
- The program accepts only a blank operation, `A`, or `0`; any delete or modify operation (`E`, `1`, `M`, `2`) produces an error.
- `qn` and `qm` are converted to per-unit values by dividing by `base`.
- `sh` is also converted to a per-unit value by dividing by `base`.

## DLIN

### Purpose

Define the circuits in the AC system.

### Records

1. The `DLIN` code.
2. One or more circuit records.
3. The `9999` record marks the end of the block.

### Line format

| Field | Columns | Description |
|-------|---------|-------------|
| `from` | 1-4 | Origin bus. |
| `operation` | 6 | Circuit operation. A blank, `A`, or `0` is accepted; `E`/`1`/`M`/`2` produce an error. |
| `to` | 9-12 | Destination bus. |
| `r` | 17-22 | Resistance in percent (`0.01 * atof(...)` if provided). |
| `x` | 23-28 | Reactance in percent (`0.01 * atof(...)` if provided). |
| `b` | 29-34 | Susceptance, divided by `base` when provided (`atof(data) / base`). |
| `tap` | 35-39 | Transformer tap. |
| `tmn` | 40-44 | Minimum tap limit. |
| `tmx` | 45-49 | Maximum tap limit. |

### Notes

- If a resistance, reactance, or susceptance field is blank, the value read is `0.`.
- The program also interprets `tap`, `tmn`, and `tmx` as floating-point values, without additional validation.
- The code reads `tmn` twice from the same part of the file; the final behavior is unchanged.

## DGER

### Purpose

Read the active generation limits for each generator.

### Records

1. The `DGER` code.
2. One or more generation records.
3. The `9999` record marks the end of the block.

### Line format

| Field | Columns | Description |
|-------|---------|-------------|
| `id` | 1-4 | Generator number (bus identifier). |
| `pmn` | 8-12 | Minimum active generation limit. If blank, defaults to `0.`; divided by `base`. |
| `pmx` | 14-18 | Maximum active generation limit. If blank, defaults to `0.`; divided by `base`. |

The parser stores these values in the `dger_t` structure and increments `nger`.

## DGLT

### Purpose

Group voltage-limit block, according to the FLUPOT specification.

### Implementation status

The parser recognizes the `DGLT` code, but does not read or store its fields in structures. It only scans the lines until it finds the `99` terminator:

```c
if (strncmp(line, "99", 2) == 0) {
  break;
}
```

Therefore, the block is accepted so that the file can continue to be read, but `DGLT` data is not fully implemented.

## DGBT

### Purpose

Base-voltage group block.

### Implementation status

As with `DGLT`, the parser recognizes `DGBT` but does not interpret the record fields. It only skips lines until it finds the `99` terminator.

Therefore, in this program, `DGBT` is not converted to internal data or used in calculations.

## FIM

### Purpose

End reading the input file.

When the `FIM` code is identified, the `input_data()` function returns `0`, indicating that reading completed successfully.

## Minimal file example

```text
TITU
Example system
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

This example illustrates the general block structure; actual data is processed by column position, not by free-text field names.
