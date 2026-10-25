# Fast-Decoupled AC Power-Flow Solver

This document explains the fast-decoupled AC power-flow method and how it is implemented in `src/fpot.c`. The input record layout and parser behavior are documented separately in [manual.md](manual.md).

## Problem solved

An AC power-flow calculation finds the steady-state voltage magnitude and phase angle at each bus that satisfy the network equations and the specified active and reactive power injections.

For bus `i`, let the voltage phasor be

`V_i = |V_i| exp(j theta_i)`

and let the bus-admittance matrix entry be

`Y_ik = G_ik + j B_ik`.

The calculated active and reactive power injections are

```text
P_i = |V_i| sum_k |V_k| [G_ik cos(theta_i - theta_k)
                         + B_ik sin(theta_i - theta_k)]

Q_i = |V_i| sum_k |V_k| [G_ik sin(theta_i - theta_k)
                         - B_ik cos(theta_i - theta_k)]
```

The specified net injection is generation minus load. A positive value denotes injection into the network; a negative value denotes net consumption. The solver iteratively adjusts voltage angles and magnitudes until the active and reactive mismatches meet their configured tolerances.

## Fast-decoupled approximation

The full Newton power-flow method couples active-power mismatch to both voltage magnitude and angle, and reactive-power mismatch to both angle and magnitude. For typical high-voltage transmission networks, resistance is small compared with reactance, angle differences are modest, and voltage magnitudes are near 1 per unit. Under these conditions, the dominant relationships are approximately:

```text
active-power mismatch   <-> voltage-angle correction
reactive-power mismatch <-> voltage-magnitude correction
```

The fast-decoupled method neglects the weaker cross-coupling and holds two coefficient matrices fixed throughout the iteration. In this implementation, the linear systems are

```text
B1 * delta_theta = delta_P / |V|
B2 * delta_V     = delta_Q / |V|
```

where the mismatch vectors are calculated from the current state. The division by voltage magnitude is performed when `dp` and `dq` are formed in `updatedp()` and `updatedq()`.

This approximation reduces work per iteration compared with repeatedly forming and factoring a full Newton Jacobian. It is most appropriate for networks that meet the assumptions above; it is not a guarantee of convergence for every case.

## Network model and matrix construction

### Full admittance terms

`formyb()` clears the admittance and susceptance arrays, then processes each input line. For a line with input resistance `r`, reactance `x`, and the command-line angle `phi`, it first rotates the series impedance:

```text
r' = r cos(phi) - x sin(phi)
x' = r sin(phi) + x cos(phi)
```

The resulting series admittance is

```text
g =  r' / (r'^2 + x'^2)
b = -x' / (r'^2 + x'^2)
```

Here `g` and `b` are the real and imaginary parts of `1 / (r' + j x')`. The code adds this admittance to the diagonal entries for the two end buses and its negative to the off-diagonal entries. Half of the line charging susceptance and the bus shunt value are also included in the diagonal terms of `yg` and `yb`.

The full arrays `yg` and `yb` are then used in the power equations. `updatedp()` calculates `p[i]` and the active mismatch; `updatedq()` calculates `q[i]` and the reactive mismatch.

### Fixed decoupled matrices

`formyb()` also builds two susceptance-based arrays for the correction equations:

- `yb1` uses `-1 / x'` for each line's series contribution, with the opposite sign in the off-diagonal positions. This omits line charging and bus shunts.
- `yb2` uses the series susceptance `b` from the full line model and adds `2 * (half line charging + bus shunt)` to each incident bus's diagonal contribution. Its off-diagonal series terms have the opposite sign.

`formb()` selects the bus subsets and negates these arrays to create the positive-oriented coefficient matrices `b1` and `b2` used by the linear solves. These are fixed for the whole power-flow iteration; the full `yg` and `yb` arrays are used to recalculate power mismatches at each step.

The construction above describes the code's actual formulas. In particular, `b2` should not be assumed to be a general-purpose, textbook Jacobian or identical to every published fast-decoupled variant.

### Factorization and linear solves

`formb()` performs an in-place elimination on `b1` and `b2` once, before iteration. It stores elimination multipliers below each diagonal and the resulting upper-triangular entries on and above the diagonal; it does not pivot rows.

For each mismatch vector, `getdth()` or `getdv()` applies forward substitution using the stored multipliers, then backward substitution through the upper-triangular matrix. The resulting correction is written to `dth` or `dv`. Because the matrices are reused, this avoids refactorization on each iteration.

## Bus types and state variables

The parser stores each bus type in `dbar[].type`; see [manual.md](manual.md#dbar) for the input field details. `formb()` uses these rules:

- **Slack (type 2):** excluded from `b1` and therefore held at its input voltage magnitude and angle. Its injection is not specified by the solver.
- **PV (type 1):** included in `b1`, so its angle is corrected; excluded from `b2`, so its voltage magnitude remains at its input value.
- **PQ (type 0):** included in both matrices; both angle and magnitude can be corrected.

Initial voltage magnitudes and phase angles are copied directly from the input bus records in `main()`. The input parser converts the bus phase from degrees to radians. No separate flat-start initialization is applied.

The parser also recognizes type 3 and counts it in `npq`. However, `formb()` includes every non-type-2 bus in the angle matrix but includes only type-0 buses in the voltage matrix. Therefore, type 3 does not follow the ordinary PQ handling described above and can make the expected reactive-equation count differ from the selected voltage-bus count. Treat type 3 cases as unsupported unless this behavior is deliberately corrected and validated.

## Specified injections and the `phi` argument

The command line is:

```text
fpot filename phi
```

`phi` is supplied in degrees and converted to radians. It is used both to rotate each line's resistance/reactance, as described above, and to rotate specified net active/reactive injections before comparing them with calculated network power.

For net active and reactive injections `P_net = pg - pl` and `Q_net = qg - ql`, `updatedp()` and `updatedq()` use:

```text
P_spec = (P_net cos(phi) - Q_net sin(phi)) / base
Q_spec = (P_net sin(phi) + Q_net cos(phi)) / base
```

`P_net` and `Q_net` are in the input's power units; `base` is the system power base from `DCTE` or its default. The mismatch terms are:

```text
dp[i] = (P_spec[i] - p[i]) / |V_i|
dq[i] = (Q_spec[i] - q[i]) / |V_i|
```

The solver compares the absolute mismatch values against `tepa` and `tepr`, respectively. Defaults are `1 / base`; the `DCTE` block can override them.

## Iteration sequence

After parsing the case, `main()` builds the admittance arrays and factors `b1` and `b2`. It then performs these steps, up to `acit` iterations:

1. **Active-power/angle step:** if the active equations have not been marked converged, calculate active injections and mismatches, solve for `dth`, and add the corrections to the angles of all buses in `b1`.
2. **Reactive-power/voltage step:** if the reactive equations have not been marked converged, calculate reactive injections and mismatches using the current state, solve for `dv`, and add the corrections to the voltage magnitudes of buses in `b2`.
3. **Convergence checks:** mark each subsystem converged when all of its selected buses have the previously calculated absolute mismatch less than or equal to the corresponding tolerance. These mismatches were calculated immediately before applying that subsystem's correction, not recomputed after it. The overall loop ends when both subsystems are marked converged or the iteration limit is exceeded.

The code maintains separate active and reactive convergence flags. Once a subsystem is marked converged, it is not recalculated on subsequent iterations while the other subsystem continues.

## Output and interpretation

On completion, the program prints an iteration count, then voltage magnitude and phase angle for each bus, followed by bus names and calculated powers scaled by `base` and rotated back using `phi`. The power values come from the most recent calls that calculated `p` and `q`; they can reflect a state from before the latest correction. The voltage report's first column is the zero-based array index, not the input bus ID.

Reaching the iteration limit is not reported as a failure: the loop exits and the program still prints the current state and returns success. Since the limit check occurs after incrementing the counter, the printed count can be `acit + 1` when the limit is exceeded. A successful process exit alone therefore does not establish convergence. Check for both active and reactive convergence messages, and review the reported iteration count and residual behavior when assessing a result.

## Scope and limitations

- This implementation is a compact educational solver, not a general-purpose power-system analysis package.
- The `DGER` records are parsed, but generator active-power limits are not enforced by the solver.
- Reactive-power limits, PV-to-PQ switching, transformer tap adjustments, and controls are not implemented in the iteration.
- Tap fields are parsed but are not used in the admittance calculations.
- The matrix elimination does not pivot or report singular matrices. The input network must yield usable, nonzero pivots for the selected bus sets.
- Matrix and state arrays have fixed capacity (`LENGHT`, currently 1000); input sizes must remain within the implementation's limits.
- The return status does not distinguish convergence from hitting the iteration limit.
- `make check` currently has no automated solver tests. The bundled cases in `examples/` can be used for smoke testing; for example, after building, run `./src/fpot examples/sis9.dad 0`.
