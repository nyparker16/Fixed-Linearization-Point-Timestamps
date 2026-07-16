---- MODULE TimestampedAtomicAbstract ----
(***************************************************************************)
(* Abstract specification of TimestampedAtomic<T>'s intended behavior:     *)
(*   - load()  returns the most recently written value.                   *)
(*   - store(v) unconditionally sets the value to v.                      *)
(*   - CAS(expected, desired) sets the value to desired only if the       *)
(*     current value equals expected; otherwise it has no effect.         *)                                                                         *)                                                       *)
(***************************************************************************)

EXTENDS Integers, Sequences, TLC

CONSTANTS
    Threads,        \* set of thread ids issuing operations
    Values,         \* finite set of values operations may store
    InitVal,        \* initial value
    OpsPerThread    \* number of operations each thread performs

(* --algorithm TimestampedAtomicAbstract

variables
    value       = InitVal,
    loadResult  = [t \in Threads |-> InitVal],
    casResult   = [t \in Threads |-> FALSE],
    opsLeft     = [t \in Threads |-> OpsPerThread];

process Thr \in Threads
variables pickVal = InitVal, pickExpected = InitVal, pickDesired = InitVal;
begin
P1:     while opsLeft[self] > 0 do
            either
                \* load: return the current value
                loadResult[self] := value;
            or
                \* store: unconditionally overwrite
                with v \in Values do
                    pickVal := v;
                end with;
                value := pickVal;
            or
                \* CAS: overwrite only if current value matches expected
                with e \in Values, d \in Values do
                    pickExpected := e;
                    pickDesired  := d;
                end with;
                if value = pickExpected then
                    value := pickDesired;
                    casResult[self] := TRUE;
                else
                    casResult[self] := FALSE;
                end if;
            end either;
P2:         opsLeft[self] := opsLeft[self] - 1;
        end while;
end process;

end algorithm; *)
====
