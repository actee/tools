# A Turing-like parsing machine

### Made with C, prints current state

---

A Turing machine, made with C, prints current state and word state

---

Usage:

`turing <fname> <input line>`

with the given example (`t_states`):

`turing t_states d001100d`

---

Takes a filename with turing state instructions, such in the format:

`<state_number>:<input><output><+/-/N><next>,<input><out(...)`

`<LF>` -> line feed or '\n'

`<state_number>` as a number that identifies the state

`<input>` & `<output>` what reads and what writes

`<+/-/N>` move right, move left or 'N' final state

`<next>` the next state, identified by its `<state_number>`

---

These instruction lines should be separated by any white-space characted ( a line feed recommended ).

Although, there should be no white-spaces in the instructions

There is an example given
