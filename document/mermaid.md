# Minishell — Programming Flow Chart

This document contains a detailed Mermaid flowchart describing the full execution flow of the minishell project, from program start to command execution and back to the prompt loop.

GitHub renders Mermaid diagrams automatically inside `.md` files — no extra tooling needed. You can also paste the code block into [mermaid.live](https://mermaid.live) to preview or export it as an image.

---

## Full Flow

```mermaid
flowchart TD
    Start(["Program start: main()"]) --> Init["init_shell: convert envp into t_env"]
    Init --> Prompt["shell_loop: readline displays prompt"]
    Prompt --> CheckEOF{"Ctrl-D pressed?"}
    CheckEOF -->|Yes| CleanExit["free env + clear history"]
    CleanExit --> Exit(["Program ends: return exit_status"])
    CheckEOF -->|No| CheckEmpty{"Input is empty?"}
    CheckEmpty -->|Yes| Prompt
    CheckEmpty -->|No| AddHistory["add_history(input)"]

    subgraph ParserPart["PARSER"]
        AddHistory --> Lexer["Lexer: tokenize into WORD, PIPE, REDIR"]
        Lexer --> QuoteCheck{"All quotes closed?"}
        QuoteCheck -->|No| ErrQuote["error: unclosed quotes"]
        QuoteCheck -->|Yes| SyntaxCheck["Syntax checker validates token list"]
        SyntaxCheck --> SyntaxValid{"Syntax valid?"}
        SyntaxValid -->|"No, e.g. double pipe or redirect with no target"| ErrSyntax["error: syntax error near token"]
        SyntaxValid -->|Yes| Expansion["Expansion: substitute VAR and exit status"]
        Expansion --> HasHeredoc{"Command contains heredoc?"}
        HasHeredoc -->|Yes| HeredocLoop["readline line by line until delimiter matches"]
        HeredocLoop --> HeredocSig{"Ctrl-C during heredoc read?"}
        HeredocSig -->|Yes| Prompt
        HeredocSig -->|No| CmdBuilder
        HasHeredoc -->|No| CmdBuilder["Command builder: build t_cmd list with redirections"]
    end

    ErrQuote --> SetErrStatus["set exit status = 2"]
    ErrSyntax --> SetErrStatus
    SetErrStatus --> Prompt

    subgraph ExecutorPart["EXECUTOR"]
        CmdBuilder --> HasPipe{"More than 1 command in pipeline?"}
        HasPipe -->|Yes| SetupPipes["create pipe() between each command"]
        HasPipe -->|No| Dispatcher
        SetupPipes --> Dispatcher["Dispatcher: iterate each command in list"]

        Dispatcher --> IsBuiltin{"Is builtin command?"}
        IsBuiltin -->|"Yes, and not inside a pipe"| RunBuiltinDirect["run builtin directly in parent process"]
        IsBuiltin -->|"Yes, but inside a pipe"| ForkBuiltin["fork() then run builtin in child"]
        IsBuiltin -->|No| ForkExternal["fork() for external command"]

        RunBuiltinDirect --> WhichBuiltin{"Select builtin"}
        WhichBuiltin -->|echo| BEcho["print args according to -n flag"]
        WhichBuiltin -->|cd| BCd["chdir and update PWD/OLDPWD"]
        WhichBuiltin -->|pwd| BPwd["print current working directory"]
        WhichBuiltin -->|export| BExport["add or update environment variable"]
        WhichBuiltin -->|unset| BUnset["remove environment variable"]
        WhichBuiltin -->|env| BEnv["print all environment variables"]
        WhichBuiltin -->|exit| BExit["validate argument then call exit_shell"]

        ForkBuiltin --> Redirect1["Redirection: open files + dup2 fds"]
        ForkExternal --> Redirect2["Redirection: open files + dup2 fds"]

        Redirect1 --> RunBuiltinChild["execute_builtin inside child process"]
        Redirect2 --> PathSearch["search executable path from PATH env"]
        PathSearch --> PathFound{"Executable found?"}
        PathFound -->|No| Err127["error: command not found, exit 127"]
        PathFound -->|Yes| ExecCall["execve()"]
        ExecCall --> ExecFail{"execve succeeded?"}
        ExecFail -->|No| Err126["error: permission denied, exit 126"]

        RunBuiltinChild --> ChildExit["child process exits with resulting status"]
        Err127 --> ChildExit
        Err126 --> ChildExit

        ChildExit --> WaitPid["parent calls waitpid for all children"]
        BEcho --> CollectStatus["collect exit status of the command"]
        BCd --> CollectStatus
        BPwd --> CollectStatus
        BExport --> CollectStatus
        BUnset --> CollectStatus
        BEnv --> CollectStatus
        BExit --> ExitShellCall["call exit_shell immediately, skip rest of pipeline"]
        ExitShellCall --> CleanExit

        WaitPid --> SignalDuring{"Signal received while child is running?"}
        SignalDuring -->|Ctrl-C| SigIntChild["send SIGINT to child, print newline, exit 130"]
        SignalDuring -->|"Ctrl-Backslash"| SigQuitChild["child core dumps, exit 131"]
        SignalDuring -->|No signal| CollectStatus
        SigIntChild --> CollectStatus
        SigQuitChild --> CollectStatus
    end

    CollectStatus --> UpdateExit["set exit status = last command's exit code in pipeline"]
    UpdateExit --> Cleanup["free t_cmd list, close all file descriptors"]
    Cleanup --> Prompt
```

---

## Diagram Structure Explained

### Subgraph ownership (maps to team split)

| Section | Owner | Covers |
|---|---|---|
| Outside any subgraph | Shared (Core) | readline, init, cleanup — start and end point of every loop iteration |
| `ParserPart` | Person 1 | Lexer through Command builder |
| `ExecutorPart` | Person 1 (builtins only) + Person 2 (everything else) | Dispatcher, pipes, redirection, path search, signals |

### Key decision points (diamonds)

1. **`SyntaxValid`** — if syntax is invalid, the turn ends immediately without ever reaching the Executor.
2. **`IsBuiltin`** — the handoff point where Parser output determines which execution path the Executor takes.
3. **`SignalDuring`** — where signal handling interrupts the parent while it waits on a running child process.

### Loop behavior

Every path — success, error, or signal interruption — loops back to `Prompt`, except two terminal cases:
- `Ctrl-D` at the prompt → `CleanExit` → `Exit`
- Builtin `exit` being called → `ExitShellCall` → `CleanExit` → `Exit`

### Exit code reference embedded in the flow

| Path | Exit code |
|---|---|
| Syntax or quote error | 2 |
| Command not found | 127 |
| Permission denied | 126 |
| Killed by `Ctrl-C` (SIGINT) | 130 |
| Killed by `Ctrl-\` (SIGQUIT) | 131 |
| Normal completion | Exit code of last command in pipeline |

---

## Sub-flow: Heredoc (zoomed in)

```mermaid
flowchart TD
    A["Command builder detects << token"] --> B["Open temp file or buffer"]
    B --> C["readline prompt: > "]
    C --> D{"Line equals delimiter?"}
    D -->|No| E{"Line contains $VAR and delimiter not quoted?"}
    E -->|Yes| F["Expand variable in line"]
    E -->|No| G["Write line as-is"]
    F --> G
    G --> H["Write line to temp file"]
    H --> C
    D -->|Yes| I["Close temp file"]
    I --> J["Attach temp file as command's stdin"]
```

## Sub-flow: Pipe Setup (zoomed in)

```mermaid
flowchart TD
    A["Command list has N commands"] --> B["Create N-1 pipe() pairs"]
    B --> C["fork() child for command i"]
    C --> D{"i has previous command?"}
    D -->|Yes| E["dup2 read-end of previous pipe to stdin"]
    D -->|No| F{"i has next command?"}
    E --> F
    F -->|Yes| G["dup2 write-end of current pipe to stdout"]
    F -->|No| H["close all unused pipe fds"]
    G --> H
    H --> I["execute command i"]
    I --> J{"More commands remaining?"}
    J -->|Yes| C
    J -->|No| K["parent closes all pipe fds"]
    K --> L["waitpid for every child"]
```