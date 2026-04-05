# Pish — A Minimal Unix Shell in C

A lightweight Unix shell implementation written in C, built from scratch to understand how real shells work internally — from lexical analysis to process execution.

---

## 📌 Overview

**Pish** is a custom shell that supports:

* Command execution via `fork` + `exec`
* Pipelines (`|`)
* I/O redirection (`<`, `>`, `>>`)
* Logical operators (`&&`, `||`)
* Background processes (`&`)
* Built-in commands (`cd`, `pwd`, `exit`)
* Custom prompt with working directory

The project is structured to mirror how real shells work internally:

```
Input → Lexer → Parser → Executor → OS
```

---

## ⚙️ Features

### 🔹 1. Command Execution

* Uses `fork()` and `execvp()` to run programs 
* Supports arbitrary Unix commands

---

### 🔹 2. Pipelines

```bash
ls | grep file | wc -l
```

* Dynamically constructs pipes between commands
* Uses `dup2()` for stream redirection 

---

### 🔹 3. I/O Redirection

```bash
cat file.txt > out.txt
grep hello < input.txt >> output.txt
```

* Supports:

  * Input: `<`
  * Output overwrite: `>`
  * Output append: `>>`
* Implemented using `open()` and file descriptors 

---

### 🔹 4. Logical Operators

```bash
make && ./program
gcc file.c || echo "Compilation failed"
```

* `&&` → execute next only if success
* `||` → execute next only if failure

---

### 🔹 5. Background Execution

```bash
sleep 10 &
```

* Runs processes without blocking the shell
* Controlled via delimiter parsing 

---

### 🔹 6. Built-in Commands

Implemented without spawning new processes:

| Command | Description             |
| ------- | ----------------------- |
| `cd`    | Change directory        |
| `pwd`   | Print working directory |
| `exit`  | Exit shell              |

Handled via function dispatch table 

---

### 🔹 7. Custom Prompt

Displays:

```
fishy@pish:[~/current/path]:%
```

* Replaces `/home/user` with `~` for readability 

---

## 🧠 Architecture

### 1. Lexer (`lexer.c`)

* Converts input string into tokens
* Handles:

  * Operators (`|`, `&&`, `||`, `>`, etc.)
  * Quoted strings
  * Text tokens

Implements a **state machine**:

* `RST` (reset)
* `CHR` (characters)
* `QTS` (quotes)
* `OPR` (operators)
* `WTE` (whitespace) 

---

### 2. Parser (`parse.c`)

* Converts tokens → structured commands
* Builds:

  * `Command` (argv + streams)
  * `Commands` (array of commands)
  * `Delims` (operators between commands)

Handles:

* Argument construction
* Redirections
* Command boundaries 

---

### 3. Executor (`exec.c`)

* Core execution engine
* Responsibilities:

  * Group commands into pipelines
  * Create pipes
  * Fork processes
  * Execute commands
  * Handle background jobs

Pipeline execution logic:

* Create `n-1` pipes for `n` commands
* Connect stdin/stdout appropriately
* Wait only if not background process 

---

### 4. Builtins (`builtin.c`)

* Avoids `fork()` for internal commands
* Uses function pointer table:

```c
{ "cd", Cd }, { "pwd", Pwd }, { "exit", Exit }
```



---

### 5. Shell Loop (`main.c`)

* Infinite REPL loop:

  1. Print prompt
  2. Read input (`getline`)
  3. Execute command
* Handles `Ctrl+C` using signal handler 

---

## 🛠️ Build & Run

### Make the build script executable

```bash
sudo chmod +x build.sh
```
### Run the build script

```bash
./build.sh
```

### Run

```bash
./pish
```

---

## 🧪 Example Usage

```bash
pish:% ls -l
pish:% cat file.txt | grep hello
pish:% gcc main.c && ./a.out
pish:% sleep 5 &
pish:% cd ..
pish:% pwd
```

---

## 📁 File Structure

```
.
├── main.c          # Shell loop
├── lexer.c         # Tokenizer
├── parse.c         # Parser
├── exec.c          # Execution engine
├── builtin.c       # Built-in commands
├── freecommand.c   # Memory cleanup
├── *.h             # Headers
```

---

## 🚧 Limitations / Future Work

* No job control (`fg`, `bg`)
* No environment variable expansion (`$PATH`, `$HOME`)
* No command history
* No advanced quoting/escaping
* No scripting support

---

## 📚 Learning Outcomes

This project helps you understand:

* Process creation (`fork`, `exec`)
* Inter-process communication (pipes)
* File descriptors and redirection
* Parsing and tokenization
* Shell design principles

---

## 🔥 Possible Extensions

If you want to push this further:

* Add **job control** (`fg`, `bg`)
* Implement **command history** (readline)
* Add **variable expansion**
* Build a **full AST instead of linear parsing**
* Add **signal handling for child processes**
* Implement **shell scripting support**

---

## 🧾 License

MIT License (or add your preferred license)
