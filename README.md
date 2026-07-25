# Minishell

โปรเจค 42 School สำหรับสร้าง shell แบบพื้นฐานด้วยภาษา C โดยพัฒนาร่วมกันเป็นทีม 2 คน แบ่งความรับผิดชอบเป็น Parser และ Executor อย่างชัดเจน เอกสารนี้สรุปภาพรวมทั้งหมดของโปรเจค ตั้งแต่แนวคิดการออกแบบ โครงสร้างไฟล์ การแบ่งงาน ไปจนถึงกติกาการทำงานร่วมกันด้วย Git

---

## สารบัญ

1. [ภาพรวมโปรเจค](#ภาพรวมโปรเจค)
2. [แนวคิดการออกแบบ](#แนวคิดการออกแบบ)
3. [โครงสร้างโปรแกรม 2 Part หลัก](#โครงสร้างโปรแกรม-2-part-หลัก)
4. [โครงสร้างไฟล์ทั้งหมด](#โครงสร้างไฟล์ทั้งหมด)
5. [การแบ่งงาน 2 คน](#การแบ่งงาน-2-คน)
6. [Git Branch Workflow](#git-branch-workflow)
7. [Dependencies](#dependencies)
8. [Feature Checklist ตาม Subject](#feature-checklist-ตาม-subject)

---

## ภาพรวมโปรเจค

Minishell คือการจำลองการทำงานของ shell (คล้าย bash) แบบพื้นฐาน โดยต้องเขียนขึ้นเองทั้งหมดตั้งแต่การอ่าน input, แยกวิเคราะห์คำสั่ง (parsing), ไปจนถึงการรันคำสั่งจริง (execution) รวมถึงจัดการ pipe, redirection, environment variable, signal และ builtin command ต่างๆ

หลักการออกแบบที่ยึดตลอดโปรเจคคือ **แยกหน้าที่ให้ชัดเจนระหว่าง "การแปลความหมาย" กับ "การลงมือทำจริง"** เพื่อให้งานสามารถแบ่งกันทำได้อย่างเป็นอิสระ ลดการชนกันของโค้ด และง่ายต่อการ debug เพราะรู้ทันทีว่าปัญหาที่เกิดขึ้นอยู่ในส่วนไหน

---

## แนวคิดการออกแบบ

โปรแกรมทั้งหมดถูกมองเป็น pipeline 4 ขั้นตอนหลัก:

```
Input → Lexer (tokenize) → Parser (build command list) → Expander → Executor
```

จากแนวคิดนี้ เราแบ่งโปรแกรมออกเป็น 2 Part ใหญ่ตามคุณสมบัติสำคัญคือ **"มีผลข้างเคียงต่อระบบจริงหรือไม่"**

- ถ้าเป็นแค่การแปลง string ให้กลายเป็นโครงสร้างข้อมูลที่พร้อมใช้งาน โดยไม่ได้เปลี่ยนแปลง state ของระบบจริง (ไม่ fork, ไม่เปลี่ยน directory, ไม่แก้ environment) → จัดอยู่ใน **Parser**
- ถ้าเป็นการลงมือทำจริงที่มีผลข้างเคียงต่อระบบ (fork process, เปิดไฟล์, เปลี่ยน current directory, แก้ไข environment variable) → จัดอยู่ใน **Executor**

จุดที่มักเข้าใจผิดคือ **Builtin command** (เช่น `cd`, `echo`, `export`) หลายคนอาจคิดว่าควรอยู่ฝั่ง Parser เพราะดูเหมือนเป็นการ "ตีความ" คำสั่ง แต่ในความเป็นจริง builtin คือคำสั่งที่ต้อง "รัน" และมีผลข้างเคียงจริง (เช่น `cd` เปลี่ยน working directory, `export` แก้ environment) เพียงแต่ไม่ต้อง `fork()` + `execve()` เหมือนโปรแกรมภายนอกเท่านั้น ดังนั้น **Builtin จึงถูกจัดให้อยู่ใน Executor เสมอ** โดยเป็นแค่หนึ่งในทางเลือกของการ execute command เท่านั้น (fork หรือไม่ fork ก็ยังถือเป็นการ "รัน" คำสั่งอยู่ดี)

---

## โครงสร้างโปรแกรม 2 Part หลัก

### 1. Parser (Front-end)

**หน้าที่:** รับ input string ดิบจากผู้ใช้ แล้วแปลงเป็นโครงสร้างข้อมูล (`t_cmd` linked-list) ที่พร้อมสำหรับส่งต่อให้ Executor นำไปรันจริง โดยไม่มีผลข้างเคียงต่อระบบ (ยกเว้น heredoc ที่ต้องอ่าน input ล่วงหน้าก่อนเริ่ม execute)

| โมดูล | หน้าที่ | รายละเอียด |
|---|---|---|
| **Lexer (Tokenizer)** | แยก string เป็น token ทีละตัว | แบ่งเป็นชนิด `WORD`, `PIPE`, `REDIR_IN`, `REDIR_OUT`, `APPEND`, `HEREDOC` ต้องจัดการ quote state (`'`, `"`) ระหว่างสแกน เพราะ `|` หรือช่องว่างที่อยู่ใน quote ไม่ถือเป็น separator จริง |
| **Syntax Checker** | ตรวจสอบความถูกต้องของ syntax | เช็ค error เช่น pipe ซ้อนกัน (`||`), ขึ้นต้น/ลงท้ายด้วย `|`, redirection ที่ไม่มี target ตามหลัง ควรแยกเป็นโมดูลต่างหากจาก lexer เพื่อให้ debug ง่าย |
| **Expansion** | แปลงตัวแปรเป็นค่าจริง | แปลง `$VAR` เป็นค่าจาก environment, แปลง `$?` เป็น exit code ล่าสุด จัดการกฎเรื่อง single quote (ไม่ expand) กับ double quote (expand ได้) |
| **Heredoc** | จัดการ `<< DELIMITER` | อ่าน input ทีละบรรทัดจนกว่าจะเจอ delimiter ที่ตรงกัน เขียนลง temp file หรือ buffer ต้องทำก่อนที่ pipeline เริ่มทำงานจริง เพราะเนื้อหาต้องพร้อมเป็น input ให้ execute |
| **Command Builder** | ประกอบ token list เป็น command list | เดิน token ทีละตัว แบ่งคำสั่งตาม `PIPE`, ผูก redirection เข้ากับ command ที่ถูกต้อง สร้างเป็น `t_cmd` linked-list พร้อม args array |

### 2. Executor (Back-end)

**หน้าที่:** รับ `t_cmd` list ที่ Parser สร้างไว้ แล้วลงมือรันจริง จัดการทุกอย่างที่เกี่ยวกับ process, file descriptor, และ system call

| โมดูล | หน้าที่ | รายละเอียด |
|---|---|---|
| **Dispatcher** | ตัดสินใจเส้นทางการรัน | เช็คว่าคำสั่งเป็น builtin หรือ external program ต้อง fork หรือไม่ (เช่น `cd` เดี่ยวๆ ไม่ fork แต่ถ้าอยู่ใน pipeline ต้อง fork เพราะ `cd` เปลี่ยน state ของ shell process เอง) |
| **Pipe Manager** | เชื่อมต่อคำสั่งใน pipeline | สร้าง `pipe()` ระหว่างแต่ละคำสั่ง, `fork()` ต่อคำสั่ง, ปิด file descriptor ที่ไม่ใช้ (จุดที่มักเกิด fd leak มากที่สุด) |
| **Redirection Handler** | จัดการการรับ-ส่งข้อมูลจากไฟล์ | เปิดไฟล์ตาม redirection ของแต่ละคำสั่ง แล้ว `dup2()` เข้า stdin/stdout ต้อง backup และ restore fd เดิมสำหรับกรณี builtin ที่รันใน parent process โดยไม่ fork |
| **Path Search** | หาตำแหน่งไฟล์ execute | ค้นหา path จาก environment variable `PATH`, เช็ค permission, แยก error case (command not found, permission denied, is a directory) |
| **Run Command** | รันโปรแกรมภายนอก | `fork()` + `execve()` สำหรับคำสั่งเดี่ยว |
| **Signal Handling** | จัดการสัญญาณจากผู้ใช้ | แยก 2 โหมด — ตอน interactive (รอ readline): `Ctrl-C` แสดง prompt ใหม่, `Ctrl-\` ไม่ทำอะไร / ตอนมี child process กำลังรัน: `Ctrl-C` ส่งต่อให้ child, `Ctrl-\` ให้ child core dump ได้ ต้องสลับ handler ทุกครั้งก่อนและหลัง fork |
| **Exit Status Management** | เก็บและอัปเดตค่า `$?` | pipeline หนึ่งเส้นจะให้ exit status ตามคำสั่งสุดท้ายเท่านั้น ต้องแปลงกรณีถูก signal ฆ่า เป็น exit code แบบ 128+n |
| **Builtins** | คำสั่งที่รันในตัว shell เอง | `echo` (พร้อม option `-n`), `cd`, `pwd`, `export`, `unset`, `env`, `exit` — จัดอยู่ใน Executor เพราะมีผลข้างเคียงต่อ shell state จริง |

### Core / Shared (ใช้ร่วมกันทั้งสอง Part)

ส่วนนี้ไม่ได้เป็นของ Part ใด Part หนึ่งเพียงฝ่ายเดียว แต่ถูกใช้งานร่วมกันจากทั้งสองฝั่ง

| ส่วน | เหตุผลที่ต้องใช้ร่วมกัน |
|---|---|
| `main.c`, `init_shell.c`, `loop.c` | เป็นจุดเชื่อมที่เรียกทั้ง Parser แล้วส่งต่อให้ Executor |
| Struct หลัก (`t_shell`, `t_cmd`, `t_env`) | ทั้งสอง Part ต้องเข้าถึงและแก้ไขข้อมูลชุดเดียวกัน |
| Environment struct (`t_env`) | ฝั่ง Parser ใช้อ่านตอน expansion (`$VAR`), ฝั่ง Executor ใช้เขียนตอน `export`/`unset` |
| Error printing utils | ใช้แสดง error message ในรูปแบบเดียวกันทั้งสองฝั่ง |
| libft | ใช้เป็นพื้นฐานทั้งโปรเจค |

---

## โครงสร้างไฟล์ทั้งหมด

เลือกใช้โครงสร้างแบบ **Flat File** คือไม่มี subfolder ย่อยใน `srcs/` แต่ใช้ **prefix ของชื่อไฟล์** แทนการแบ่ง folder ตาม pattern:

```
<part>_<module>_<detail>.c
```

ข้อดีของวิธีนี้คือเมื่อ `ls srcs/ | sort` ไฟล์กลุ่มเดียวกันจะเรียงติดกันโดยอัตโนมัติ ค้นหาด้วย `grep` หรือ fuzzy finder ได้ง่าย และไม่ต้องกังวลเรื่อง include path ที่ซ้อนกันหลายชั้น

```
srcs/
│
├── main.c                        # entry point
├── init_shell.c                  # สร้าง/init t_shell struct
├── loop.c                        # shell loop หลัก (readline → parse → execute)
│
├── parser_lexer.c                # tokenize string → token list
├── parser_lexer_quotes.c         # จัดการ quote state ระหว่าง lex
├── parser_lexer_utils.c          # helper: is_separator, is_space ฯลฯ
├── parser_syntax_check.c         # เช็ค syntax error
├── parser_expansion.c            # expand $VAR, $?
├── parser_expansion_utils.c      # helper สำหรับ expansion
├── parser_heredoc.c              # จัดการ heredoc <<
├── parser_heredoc_expand.c       # expand ตัวแปรใน heredoc
├── parser_cmd_builder.c          # เดิน token list → สร้าง t_cmd list
├── parser_cmd_builder_utils.c    # helper สำหรับสร้าง args/redir list
│
├── exec_dispatcher.c             # ตัดสินใจ builtin/external, จัดการ pipeline
├── exec_pipes.c                  # สร้าง pipe(), fork() ต่อ cmd
├── exec_redirection.c            # เปิดไฟล์ + dup2 ตาม redir list
├── exec_path.c                   # หา path จาก PATH env, เช็ค permission
├── exec_run_cmd.c                # fork + execve คำสั่งเดียว
├── exec_signals.c                # signal handler (interactive vs child running)
├── exec_exit_status.c            # เก็บ/แปลง exit code, $?
│
├── builtin_echo.c
├── builtin_cd.c
├── builtin_pwd.c
├── builtin_export.c
├── builtin_export_utils.c
├── builtin_unset.c
├── builtin_env.c
├── builtin_exit.c
│
├── env_init.c                    # แปลง envp[] → internal struct
├── env_get.c                     # get_env_value, get_env_index
├── env_set.c                     # set_env_var, remove_env_var
│
├── free_utils.c                  # free token list, cmd list, env list
└── error_utils.c                 # print error message ตาม format มาตรฐาน
```

### ตาราง Mapping Prefix → Part

| Prefix | Part | คำอธิบาย |
|---|---|---|
| (ไม่มี prefix) | Core | เชื่อม Parser กับ Executor เข้าด้วยกัน |
| `parser_*` | Parser | lexer → syntax check → expansion → heredoc → cmd builder |
| `exec_*` | Executor | dispatch, pipe, redirect, execve, signal, exit status |
| `builtin_*` | Executor (ย่อย) | คำสั่งในตัว แยก prefix ต่างหากเพื่อค้นหาไฟล์ง่ายขึ้น |
| `env_*` | Shared | ใช้ทั้ง Parser (expansion อ่าน) และ Executor (export/unset เขียน) |
| `free_utils`, `error_utils` | Shared/Utils | ใช้ทั้งโปรเจค |

### ตัวอย่าง Makefile SRC list

```makefile
# Core
SRC_CORE = main.c init_shell.c loop.c

# Parser
SRC_PARSER = parser_lexer.c parser_lexer_quotes.c parser_lexer_utils.c \
             parser_syntax_check.c \
             parser_expansion.c parser_expansion_utils.c \
             parser_heredoc.c parser_heredoc_expand.c \
             parser_cmd_builder.c parser_cmd_builder_utils.c

# Executor
SRC_EXEC = exec_dispatcher.c exec_pipes.c exec_redirection.c \
           exec_path.c exec_run_cmd.c exec_signals.c exec_exit_status.c

# Builtins
SRC_BUILTIN = builtin_echo.c builtin_cd.c builtin_pwd.c \
              builtin_export.c builtin_export_utils.c \
              builtin_unset.c builtin_env.c builtin_exit.c

# Environment (shared)
SRC_ENV = env_init.c env_get.c env_set.c

# Utils
SRC_UTILS = free_utils.c error_utils.c

SRC = $(SRC_CORE) $(SRC_PARSER) $(SRC_EXEC) $(SRC_BUILTIN) $(SRC_ENV) $(SRC_UTILS)
SRCS = $(addprefix srcs/, $(SRC))
OBJS = $(SRCS:.c=.o)
```

---

## การแบ่งงาน 2 คน

| คนที่ 1 | คนที่ 2 |
|---|---|
| **Parser ทั้งหมด**: lexer, syntax check, expansion, heredoc, cmd builder | **Executor core ทั้งหมด**: dispatcher, pipes, redirection, path search, run_cmd, signals, exit status |
| **Builtins ทั้งหมด**: echo, cd, pwd, export, unset, env, exit | |

### เหตุผลของการแบ่งแบบนี้

Builtin แม้จะจัดอยู่ใน Part Executor ตามหลักการออกแบบ แต่ในทางปฏิบัติ builtin **ไม่ต้องยุ่งกับ fork/pipe โดยตรง** (ยกเว้นตอนที่ builtin ต้องรันในบริบท pipeline ซึ่งฝั่ง dispatcher ของคนที่ 2 จะเป็นผู้ตัดสินใจว่าต้อง fork หรือไม่) จึงเป็นงานที่ค่อนข้าง self-contained แยกออกจากกลไกหนักอย่าง fork/pipe/signal ได้ การให้คนที่ 1 ถือ Parser และ Builtin ไปพร้อมกันทำให้คนที่ 2 โฟกัสเฉพาะกลไกที่ซับซ้อนที่สุดของ Executor ได้เต็มที่

### จุดเชื่อมต่อสำคัญที่ต้องตกลงกันก่อนแยกงาน (Interface Contract)

ก่อนแยกไปทำคนละทาง ทั้งคู่ต้องตกลง struct ร่วมกันให้ชัดเจนก่อน เพราะเป็นจุดที่ทั้งสอง Part ต้องมาเจอกัน:

```c
typedef struct s_redir
{
    int             type;   // REDIR_IN, REDIR_OUT, APPEND, HEREDOC
    char            *file;  // หรือ heredoc delimiter
    struct s_redir  *next;
}   t_redir;

typedef struct s_cmd
{
    char            **args;      // args[0] = ชื่อคำสั่ง
    t_redir         *redirs;     // list ของ redirection ก่อนรันคำสั่งนี้
    struct s_cmd    *next;       // คำสั่งถัดไปใน pipeline
}   t_cmd;
```

คำถามที่ต้องตกลงร่วมกันก่อนเริ่มแยกงาน:

- `t_cmd->args[0]` เป็น builtin ตัวไหน Executor จะรู้ได้อย่างไรว่าต้องเรียก builtin function ใด (เทียบ string ตรงๆ หรือแปลงเป็น enum)
- Heredoc เก็บเนื้อหาไว้ที่ไหน — เป็น temp filename ใน `t_redir` หรือเก็บเป็น file descriptor ที่เปิดค้างไว้เลย
- Exit status (`$?`) เก็บแบบ global variable หรือส่งผ่าน pointer ของ `t_shell` ในทุกฟังก์ชัน

---

## Git Branch Workflow

### โครงสร้าง Branch

ใช้แนวทาง **Trunk-Based Development แบบง่าย** เหมาะกับทีมขนาดเล็กที่สื่อสารกันได้ตลอดเวลา ไม่ใช้โครงสร้างแบบ Git Flow ที่มี `dev` คั่นกลางและ feature branch จำนวนมาก เพราะเป็น overhead ที่ไม่จำเป็นสำหรับทีม 2 คน

```
main       ← โค้ดที่ compile ผ่านเสมอ ห้ามพัง
 ├── parser     (คนที่ 1: parser + builtin)
 └── executor   (คนที่ 2: executor core)
```

แต่ละคนมี branch หลักของตัวเองเพียง **1 อัน** ทำงานยาวต่อเนื่องในนั้น ไม่ต้องแยกย่อยเป็น feature branch จำนวนมาก ยกเว้นกรณีต้องการทดลองสิ่งที่มีความเสี่ยงจะทำโค้ดเดิมพัง จึงค่อยแตก branch ทดลองแยกออกไปชั่วคราว

### กฎการทำงาน 8 ข้อ

**1. คนละ branch คนละหน้าที่**
คนที่ 1 ทำงานใน branch `parser` เท่านั้น (ครอบคลุม lexer, syntax check, expansion, heredoc, cmd builder, และ builtin ทั้งหมด) คนที่ 2 ทำงานใน branch `executor` เท่านั้น (dispatcher, pipes, redirection, path, run_cmd, signals, exit status)

**2. ห้าม push ตรงเข้า `main`**
ทำงานใน branch ของตัวเองก่อนเสมอ ตรวจสอบว่า compile ผ่านแล้วจึงค่อย merge เข้า `main`

**3. Pull จาก `main` ทุกวัน อย่างน้อยวันละครั้ง**

```bash
git checkout main
git pull
git checkout parser        # หรือ executor
git merge main
```

ยิ่งปล่อยให้ branch ห่างจาก `main` นานเท่าไหร่ ยิ่งมีโอกาส conflict สะสมมากขึ้น การ merge บ่อยๆ ทีละน้อยจะทำให้แต่ละครั้งที่เจอ conflict มีขนาดเล็กและแก้ง่ายกว่า

**4. Commit บ่อยๆ ทีละชิ้นเล็ก**
หลักการคือ 1 commit ควรอธิบายได้ในประโยคเดียวว่าทำอะไร ห้าม commit ก้อนใหญ่ที่รวมหลายงานเข้าด้วยกัน เพราะจะทำให้ revert หรือหาจุดที่ทำให้เกิดบั๊กทำได้ยาก

**5. Commit message ต้องมี prefix บอกส่วนงาน**

```
parser: add quote-aware tokenizer
builtin: implement export without args
exec: fix fd leak in pipe close
fix: resolve double-free in free_tokens
```

**6. Merge เข้า `main` เฉพาะตอนที่ compile ผ่านแบบไม่มี warning**
ใช้ flag `-Wall -Wextra -Werror` เช็คก่อน merge ทุกครั้ง ห้ามปล่อยโค้ดที่ compile ไม่ผ่านเข้า `main` เด็ดขาด เพราะจะกระทบการทำงานของอีกฝ่ายทันทีที่ pull ไป

**7. แก้ `minishell.h` (struct หลักที่ใช้ร่วมกัน) ต้องแจ้งอีกฝ่ายก่อนเสมอ**
struct อย่าง `t_cmd`, `t_env`, `t_shell` เป็นจุดที่ทั้งสองฝั่งอ้างอิงร่วมกัน การเพิ่มหรือแก้ field ใดๆ ต้องคุยกันก่อนเสมอ ห้ามแก้เงียบๆ แล้ว push ขึ้นไปตรงๆ

**8. เจอ merge conflict ให้คุยกันสด ไม่เดาเอาเอง**
เมื่อเกิด conflict ห้ามเลือกเก็บโค้ดฝั่งใดฝั่งหนึ่งทิ้งไปโดยไม่ปรึกษาเจ้าของโค้ดส่วนนั้นก่อน ควรคุยกันทันทีว่าทำไมโค้ดจุดนั้นถึงต่างกัน

### Sync Point ที่ต้องนัดคุยกันเป็นพิเศษ

| จุด | สิ่งที่ต้องเช็คร่วมกัน |
|---|---|
| ก่อนเริ่มงาน | ตกลง struct `t_cmd` / `t_env` / `t_shell` ให้นิ่งก่อนแยกไปทำคนละทาง |
| กลางทาง | ลอง merge `parser` และ `executor` เข้า `main` ครั้งแรกให้เร็วที่สุด แม้ยังไม่เสร็จสมบูรณ์ทั้งหมด เพื่อยืนยันว่าทั้งสองฝั่งเชื่อมต่อกันได้จริง |
| ก่อน merge ใหญ่ | แจ้งอีกฝ่ายก่อนเสมอว่า "กำลังจะ merge ตอนนี้" เพื่อป้องกันการ merge ชนกัน |
| ก่อนส่งงาน | ทดสอบรวมทั้งระบบร่วมกันด้วย test case จริง ครอบคลุม pipe, redirection, builtin, heredoc |

### ตัวอย่างคำสั่งที่ใช้งานจริง

```bash
# ตั้งค่าเริ่มต้นครั้งเดียว (ทำโดยคนใดคนหนึ่ง)
git init
git add .
git commit -m "initial skeleton"
git push origin main

# คนที่ 1
git checkout -b parser

# คนที่ 2
git checkout -b executor

# ทำงาน commit ไปเรื่อยๆ ระหว่างวัน
git add .
git commit -m "parser: add basic tokenizer for WORD and PIPE"

# ส่งขึ้น remote ให้อีกฝ่ายเห็น
git push origin parser

# เมื่อพร้อม merge เข้า main
git checkout main
git pull
git merge parser
git push
```

### ข้อห้าม

- ห้ามสร้าง branch ย่อยเกินความจำเป็น ยกเว้นกรณีทดลองสิ่งที่มีความเสี่ยงจะทำโค้ดเดิมพัง
- ห้ามปล่อยให้ branch ของตัวเองค้างไม่ pull จาก `main` นานเกิน 1-2 วัน
- ห้าม force push (`git push -f`) เข้า `main` โดยไม่แจ้งอีกฝ่ายก่อน
- ห้าม commit ไฟล์ที่เป็นผลลัพธ์จากการ build เช่น `.o`, `libft.a`, executable `minishell` ต้องมี `.gitignore` ป้องกันไว้ตั้งแต่ commit แรก

### ตัวอย่าง .gitignore

```gitignore
*.o
*.a
minishell
.DS_Store
```

---

## Dependencies

| Library | ประเภท | การใช้งาน |
|---|---|---|
| **libft** | เขียนเอง | ใช้เป็นพื้นฐานทั้งโปรเจค (string, memory, list operations) |
| **readline** (`-lreadline`) | External library ตัวเดียวที่อนุญาต | ใช้เฉพาะ input layer: `readline()` อ่าน input พร้อม prompt และ line-editing, `add_history()` เก็บประวัติคำสั่ง, และฟังก์ชันช่วย reset prompt เช่น `rl_on_new_line()`, `rl_replace_line()`, `rl_redisplay()` ตอนโดน signal |

**ข้อควรทราบ:** readline จัดการเฉพาะขั้นตอนการรับ input จากผู้ใช้เท่านั้น ไม่ใช่แกนหลักของ business logic ในโปรเจค ส่วนที่เป็นหัวใจจริงของ minishell คือ Parser และ Executor ที่ต้องออกแบบและเขียนเองทั้งหมด

---

## Feature Checklist ตาม Subject

- [ ] Prompt แสดงผลเมื่อรอรับคำสั่งใหม่
- [ ] Command history (เลื่อนดูคำสั่งก่อนหน้าด้วยลูกศรขึ้น/ลง)
- [ ] รันโปรแกรมจาก PATH หรือ relative/absolute path
- [ ] จัดการ single quote (`'`) และ double quote (`"`) ตามกฎของ bash
- [ ] Redirection: `<`, `>`, `>>`
- [ ] Heredoc: `<< DELIMITER`
- [ ] Pipe: `|`
- [ ] Environment variable expansion: `$VAR`
- [ ] Exit status expansion: `$?`
- [ ] Signal handling: `Ctrl-C`, `Ctrl-D`, `Ctrl-\`
- [ ] Builtin: `echo` (พร้อม option `-n`)
- [ ] Builtin: `cd` (relative/absolute path)
- [ ] Builtin: `pwd`
- [ ] Builtin: `export`
- [ ] Builtin: `unset`
- [ ] Builtin: `env`
- [ ] Builtin: `exit`
- [ ] ไม่ต้องรองรับ: `\`, `;`, `&&`, `||`, wildcard

---

## หมายเหตุท้ายเอกสาร

โครงสร้างและกติกาทั้งหมดในเอกสารนี้เป็น **living document** ที่สามารถปรับแก้ได้ระหว่างพัฒนาโปรเจคจริง โดยเฉพาะ struct หลักที่มักต้องขยายเพิ่มเมื่อลงมือ implement จริงแล้วพบว่าต้องการข้อมูลเพิ่มเติมที่ไม่ได้คาดไว้ตั้งแต่แรก แนวทางที่แนะนำคือร่างโครงสร้างแบบหยาบก่อนเพื่อให้มีจุดเริ่มต้นที่ compile ผ่าน แล้วปล่อยให้การเขียนโค้ดจริงเป็นตัวบอกว่าโครงสร้างต้องขยายไปในทิศทางใด ไม่ใช่พยายามออกแบบให้สมบูรณ์ 100% ตั้งแต่ครั้งแรก