# Minishell — แผนงาน Pipeline ฉบับละเอียด (Mandatory Part เท่านั้น)

> ทีม 2 คน — พี่: **Parser + Builtins** | เพื่อน: **Executor core**
> Reference หลัก: mcombeau (โครงสร้างชัด, comment ดี)
> Scope: เฉพาะ mandatory ของ subject 42 (ไม่มี bonus `&&` `||` `()` wildcard)

---

## 1. ภาพรวม Pipeline ทั้งระบบ

```
readline() ได้ string ดิบ
        │
        ▼
┌─────────────────────────────────────────────┐
│  PARSER (ของพี่) — ไม่มี side effect          │
│                                               │
│  [1] LEXER            → t_token list (raw)   │
│  [2] SYNTAX CHECK      → t_token list (ผ่าน) │
│  [3] EXPANSION         → t_token list (แทนค่า)│
│  [4] QUOTE REMOVAL     → t_token list (สะอาด)│
│  [5] COMMAND BUILD     → t_command list      │
└─────────────────────────────────────────────┘
        │
        ▼  ส่ง t_command* ให้ Executor
┌─────────────────────────────────────────────┐
│  EXECUTOR (เพื่อน) — มี side effect           │
│                                               │
│  [6] PREP (pipe/fd setup)                    │
│  [7] FORK + EXEC / BUILTIN (ของพี่ ผูกกับ    │
│      Executor เพราะมี side effect)           │
│  [8] WAIT + EXIT STATUS                      │
└─────────────────────────────────────────────┘
        │
        ▼
   g_exit_status  ← ใช้ต่อใน prompt รอบถัดไป ($?)
```

**กฎเหล็กของ pipeline:**
1. แต่ละ stage รับ input เป็นผลลัพธ์ของ stage ก่อนหน้าเท่านั้น ห้ามข้ามขั้น
2. Stage 1–5 (Parser) **ห้ามมี side effect** — ห้าม fork, ห้ามเปิดไฟล์จริง (heredoc เป็นข้อยกเว้นเฉพาะ เพราะต้องอ่าน input แบบ interactive — ใส่ไว้ใน stage 5 แต่ flag ไว้ชัดว่าเป็นจุดเดียวที่ parser แตะ I/O)
3. Stage 3 ต้องมาก่อน Stage 4 เสมอ (expand ก่อนลบ quote ไม่งั้นแยกไม่ออกว่าใน `'...'` ห้าม expand)
4. Builtin ถูก "ตัดสินใจ" (decide) ที่ parser ไม่ได้ — เป็นหน้าที่ Executor เพราะ criterion คือ side effect ไม่ใช่ fork/execve

---

## 2. Data Structures หลัก (ตกลงร่วมกับ Executor)

### 2.1 `t_token` — ผลลัพธ์จาก Lexer ถึง Command Build

```c
typedef enum e_token_type
{
    TOKEN_WORD,          // "ls", "-l", "$HOME", "'hello world'"
    TOKEN_PIPE,          // |
    TOKEN_REDIR_IN,      // <
    TOKEN_REDIR_OUT,     // >
    TOKEN_REDIR_APPEND,  // >>
    TOKEN_HEREDOC,       // <<
    TOKEN_END            // sentinel ปิดท้าย list เสมอ
}   t_token_type;

typedef enum e_quote_kind
{
    QK_NONE,      // ไม่มี quote เลย
    QK_SINGLE,    // ทั้ง token ห่อด้วย '...' คู่เดียวล้วน ไม่ผสมอย่างอื่น
    QK_DOUBLE,    // ทั้ง token ห่อด้วย "..." คู่เดียวล้วน ไม่ผสมอย่างอื่น
    QK_MIXED      // ผสมหลายแบบ/มีส่วนไม่ quote ปน (เช่น hello"$USER"'end')
}   t_quote_kind;

typedef struct s_token
{
    char            *value;       // เนื้อหาดิบ/หลัง expand (mutable ตาม stage)
    t_token_type    type;         // ชนิด token
    t_quote_kind    quote_kind;   // เซ็ตที่ Lexer (Stage 1) — Stage 4 ห้ามลบ field นี้ทิ้ง แม้จะลบตัวอักษร quote ออกจาก value แล้ว
    struct s_token  *prev;
    struct s_token  *next;
}   t_token;
```

**เหตุผลของแต่ละ field:**
| field | ใช้ที่ stage | เหตุผล |
|---|---|---|
| `value` | ทุก stage | เนื้อหาเปลี่ยนไปเรื่อยๆ ตาม stage (raw → expanded → unquoted) |
| `type` | Syntax Check, Command Build | ตัดสินว่า token นี้เป็น word หรือ operator |
| `quote_kind` | Expansion (fast-path), **Command Build (heredoc delimiter — จำเป็น ไม่ใช่แค่ optimization)** | เซ็ตที่ Lexer ตอน scan (รู้อยู่แล้วว่า quote เปิด/ปิดตรงไหน) แล้ว **คงอยู่ตลอด pipeline แม้ตัวอักษร quote ใน `value` จะถูกลบไปแล้วที่ Stage 4** เพราะ field นี้อยู่คนละที่กับ `value` จึงไม่หายไปพร้อมกัน |
| `prev`/`next` | ทุก stage | doubly-linked list เดินหน้า-ถอยหลังได้ (จำเป็นตอนเช็ค syntax เช่น token ก่อนหน้า `|` ต้องไม่ใช่ operator) |

> **กฎสำคัญ (แก้ไขจากรอบก่อน):** ก่อนหน้านี้เคยตัด field ระดับ token ออกโดยคิดว่าเป็น "dead field" เพราะ Stage 3 (Expansion) ต้อง char-scan ทีละตัวอักษรอยู่ดีสำหรับ `QK_DOUBLE`/`QK_MIXED` — ข้อนั้นยังจริง (`quote_kind == QK_SINGLE` ใช้เป็น fast-path ข้าม char-scan ได้เท่านั้น ส่วน `QK_DOUBLE`/`QK_MIXED` ต้อง scan หา `$` อยู่ดี) **แต่** field นี้กลับจำเป็นสำหรับอีก use-case หนึ่งที่ไม่เกี่ยวกับ Expansion เลย: **Stage 5 (heredoc)** ต้องรู้ว่า delimiter token ถูก quote หรือไม่ และ ณ ตอนนั้นตัวอักษร quote ใน `value` ถูก Stage 4 ลบไปแล้ว — ถ้าไม่มี field ที่อยู่รอดจาก Stage 4 มาถึง Stage 5 ได้ จะไม่มีทางคำนวณ `heredoc_no_expand` ได้เลย **ดังนั้น Stage 4 ต้องลบเฉพาะตัวอักษรใน `value` เท่านั้น ห้ามแตะ `quote_kind`**

### 2.2 `t_redir` — ผลลัพธ์ของ Command Build (redirection ต่อ command)

```c
typedef struct s_redir
{
    t_token_type    type;             // REDIR_IN / REDIR_OUT / REDIR_APPEND / HEREDOC
    char            *file;            // ชื่อไฟล์ (หรือ delimiter ถ้า HEREDOC)
    bool            heredoc_no_expand;// true เมื่อ delimiter ถูก quote (<< "EOF" หรือ << 'EOF') — ปิด $VAR expansion ทั้ง body
    struct s_redir  *next;            // list เพราะ 1 command มี redir ซ้อนได้ (cat < a < b > c)
}   t_redir;
```

**เหตุผล:** `cat < a < b > c` ถูกไวยากรณ์ (bash ใช้ตัวหลังสุดของแต่ละทิศทาง) → ต้องเก็บเป็น list ไม่ใช่ struct เดี่ยว ไม่งั้น parser จะ throw ข้อมูลทิ้งโดยไม่ได้ตั้งใจ

`heredoc_no_expand` เก็บไว้ที่ `t_redir` (ไม่ใช่ `t_token`) เพราะเป็น behavior ที่ผูกกับ redirection นี้โดยเฉพาะ ไม่ใช่คุณสมบัติของ token คำสั่ง — คำนวณจาก `delimiter_token->quote_kind != QK_NONE` ตอนสร้าง `t_redir` node ใน Stage 5 (อ่านค่าที่ Stage 1 เซ็ตไว้ และ Stage 4 คงไว้ไม่ลบ — ดู 2.1) — mandatory requirement ตรงๆ: `<< "EOF"` และ `<< 'EOF'` ต้องทำให้ทุกบรรทัดใน heredoc body **ไม่** ถูก expand แม้จะพิมพ์ `$USER` ธรรมดาก็ตาม ต่างจาก `<< EOF` (ไม่ quote) ที่ต้อง expand ปกติ

### 2.3 `t_command` — ผลลัพธ์สุดท้ายที่ส่งให้ Executor

```c
typedef struct s_command
{
    char                **args;        // args[0] = ชื่อ command, NULL-terminated
    t_redir             *redirs;       // list ของ redirection ทั้งหมดของ cmd นี้
    bool                is_builtin;    // ตัดสินใจที่ Command Build stage (ดู 4.5)
    struct s_command    *next;         // สำหรับ pipeline: cmd1 | cmd2 | cmd3
    struct s_command    *prev;
}   t_command;
```

**เหตุผล:** `next`/`prev` ทำให้ Executor รู้ตำแหน่งใน pipeline (คำสั่งแรก/สุดท้าย ต้องรู้เพื่อตัดสินใจว่าจะ dup2 stdin/stdout ยังไง) โดยไม่ต้องนับ index ซ้ำ

### 2.4 `t_env` — Environment (ใช้ร่วมกันทั้ง Parser และ Executor)

```c
typedef struct s_env
{
    char            *key;
    char            *value;   // NULL ได้ ถ้า export ตัวแปรแบบไม่มีค่า
    struct s_env    *next;
}   t_env;
```

ใช้ linked list แทน `char **envp` ตรงๆ เพราะ `export`/`unset` ต้อง insert/delete บ่อย ถ้าใช้ array ต้อง realloc ทุกครั้ง — list เร็วกว่าและโค้ด builtin export/unset ง่ายกว่ามาก

### 2.5 `t_heredoc_result` — ค่าที่ `run_heredoc()` คืนกลับ (Stage 5)

```c
typedef enum e_heredoc_result
{
    HEREDOC_OK,            // อ่านจนเจอ delimiter ปกติ
    HEREDOC_INTERRUPTED    // โดน SIGINT ระหว่างอ่าน ต้อง abort ทั้ง command list (ดู Stage 5)
}   t_heredoc_result;
```

---

## 3. รายชื่อไฟล์ (ยึดตาม flat + prefix ที่ตกลงไว้)

```
srcs/
  parser_lexer.c          — Stage 1
  parser_lexer_utils.c    — Stage 1 helper
  parser_syntax.c         — Stage 2
  parser_expand.c         — Stage 3
  parser_expand_utils.c   — Stage 3 helper
  parser_quote_strip.c    — Stage 4
  parser_cmd_build.c      — Stage 5
  parser_cmd_utils.c      — Stage 5 helper
  parser_heredoc.c        — Stage 5 (heredoc sub-flow)
  env_init.c              — env list init จาก envp
  env_get_set.c           — get/set/unset env
  builtin_echo.c / cd.c / pwd.c / export.c / unset.c / env.c / exit.c
  exec_*.c                — ของเพื่อน (ไม่ลงรายละเอียดในเอกสารนี้ ยกเว้นจุด integration)
```

---

## 4. รายละเอียดแต่ละ Stage

### Stage 1 — LEXER (`parser_lexer.c`)

**หน้าที่:** อ่าน string ดิบทีละตัวอักษร แยกเป็น token (word / operator) โดยยังไม่สนใจความหมาย ไม่ expand ไม่ลบ quote

**Input:** `char *raw_input` (จาก readline)
**Output:** `t_token *` เมื่อสำเร็จ, **หรือ `NULL` + error message เมื่อ quote ไม่ปิด** (token ที่สร้างไปแล้วบางส่วนต้อง free ทิ้งก่อน return)

| ฟังก์ชัน | Input | Output | หน้าที่ |
|---|---|---|---|
| `t_token *lexer(char *input)` | raw string | token list หรือ `NULL` | จุดเริ่ม วน loop ทีละตัวอักษร เรียก helper ด้านล่าง, เช็ค unclosed quote ก่อน return |
| `int lexer_next_token(char *s, int i, t_token **out)` | string + index ปัจจุบัน | token 1 ตัว + index ใหม่ | ตัดสินว่าตัวอักษร ณ ตำแหน่งนี้เริ่ม word หรือ operator แล้วเรียก scan ที่ถูกต้อง — ถ้าเป็น word ต้องเรียก `lexer_get_quote_kind()` ด้วยก่อน malloc token เพื่อเซ็ต `token->quote_kind` ให้ครบตั้งแต่สร้าง (ห้ามปล่อยเป็นค่า default เฉยๆ) |
| `char *lexer_scan_word(char *s, int *i)` | string + pointer index | สตริงคำ (ยังไม่ strip quote) | **boundary ของ token คือ whitespace ที่อยู่นอก quote เท่านั้น** ไม่ใช่แค่เจอ quote ปิดตัวเดียว — ถ้า scan เจอ quote ปิดแล้วตัวถัดไปไม่ใช่ whitespace (เช่น `hello"world"'!'`) ต้องเดินสแกนต่อรวมเป็น token เดียว ต้อง track quote state ระหว่าง scan (`'` / `"` toggle) ตลอด |
| `t_quote_kind lexer_get_quote_kind(char *word)` | string ที่ยังไม่ strip quote (ผลจาก `lexer_scan_word`) | `QK_NONE`/`QK_SINGLE`/`QK_DOUBLE`/`QK_MIXED` | ตรวจว่า `word` ทั้งก้อนถูกห่อด้วย quote คู่เดียวล้วนไหม: ตัวแรกเป็น `'`หรือ`"`, ตัวสุดท้ายเป็น quote ชนิดเดียวกันที่ปิดคู่แรกพอดี, ไม่มีอะไรอยู่นอกคู่นั้นเลย → `QK_SINGLE`/`QK_DOUBLE`; มี quote แทรกอยู่บ้างแต่ไม่เข้าเงื่อนไขนั้น → `QK_MIXED`; ไม่มี quote เลย → `QK_NONE` — **นี่คือจุดเดียวที่คำนวณ `quote_kind` ทั้ง pipeline** เรียกจาก `lexer_next_token` ทันทีหลัง `lexer_scan_word` คืนค่ามา |
| `t_token_type lexer_scan_operator(char *s, int *i)` | string + index | ชนิด operator | เช็ค `|` `<` `<<` `>` `>>` แล้วขยับ index ตามความยาว operator (1 หรือ 2 ตัวอักษร) |
| `bool lexer_is_operator_char(char c)` | 1 ตัวอักษร | bool | ใช้ตัดสินจุดสิ้นสุดของ word |
| `bool lexer_has_unclosed_quote(char *s)` | ทั้ง string ดิบ | bool | เช็คว่า quote state สุดท้ายหลัง scan จบคือ `DEFAULT` ไม่ใช่ `SQUOTE`/`DQUOTE` ค้างอยู่ |

**Data ที่ไหลผ่าน:** `char *` → ผ่าน scan ทีละตัว → ประกอบเป็น `t_token` ทีละตัว → ต่อเข้า linked list ด้วย `lst_add_back_token()`

**ตัวอย่าง (สำเร็จ):**
```
input:  echo "hello $USER" | wc -l

Lexer output (5 token):
[0] type=WORD  value=echo
[1] type=WORD  value="hello $USER"   (ยังไม่ expand ไม่ strip quote)
[2] type=PIPE  value=|
[3] type=WORD  value=wc
[4] type=WORD  value=-l
[5] type=END   value=NULL
```

**ตัวอย่าง (unclosed quote — mandatory requirement):**
```
input:  echo "hello

lexer() เดิน scan จนจบ string แล้วพบว่า quote state = DQUOTE (ไม่กลับเป็น DEFAULT)
  → free token ทั้งหมดที่สร้างไปแล้ว (echo)
  → print: minishell: unclosed quotes
  → return NULL

main loop เห็น NULL จาก lexer() → ไม่เข้า Stage 2 เลย → กลับไป prompt รอบใหม่ทันที
```

**จุดที่พลาดบ่อย (ตามที่เคยเจอ bug):**
- ต้อง scan **ทีละตัวอักษร** ห้ามใช้ `ft_split(str, ' ')` เพราะ space ที่อยู่ใน quote ต้องไม่ตัดคำ (`"hello world"` ต้องเป็น 1 token ไม่ใช่ 2)
- malloc token ใหม่ต้อง initialize `prev = NULL, next = NULL` ทุกครั้ง (bug เดิมที่เจอ: malloc init ผิด)
- ห้าม `strdup` ซ้ำสองรอบโดยไม่ตั้งใจ (เปลือง memory + memory leak เวลา free)
- หลัง scan คำเสร็จ ต้องคำนวณ substring index ให้ตรง (`start` ถึง `i` ปัจจุบัน ไม่ใช่ `i+1` หรือ `i-1` ผิด)
- **unclosed quote ต้อง return `NULL` ไม่ crash ไม่ loop ค้าง** — ถ้าไม่เช็คกรณีนี้ scan จะวิ่งเลย string length ตอนหา quote ปิดที่ไม่มีจริง

---

### Stage 2 — SYNTAX CHECK (`parser_syntax.c`)

**หน้าที่:** เดิน list ที่ได้จาก Lexer ตรวจว่าถูกไวยากรณ์ตาม subject มั้ย (ไม่แก้ไข token ใดๆ อ่านอย่างเดียว)

**Input:** `t_token *` (จาก Stage 1)
**Output:** `bool` (ผ่าน/ไม่ผ่าน) + error message ถ้าไม่ผ่าน — **list เดิมไม่เปลี่ยนแปลง**

| ฟังก์ชัน | Input | Output | หน้าที่ |
|---|---|---|---|
| `bool syntax_check(t_token *head)` | token list | true/false | วน loop เรียก check ย่อยทีละ token |
| `bool check_pipe_position(t_token *tok)` | token ปัจจุบัน | bool | `|` ต้องไม่ใช่ token แรก, ไม่ใช่ token สุดท้ายก่อน END, ไม่ติดกัน 2 อัน |
| `bool check_redir_has_target(t_token *tok)` | token ที่เป็น redir | bool | หลัง `<` `>` `>>` `<<` ต้องมี `TOKEN_WORD` ตามมาเสมอ (ไม่ใช่ END หรือ operator อื่น) |
| `void syntax_error(char *near_token)` | สตริง token ที่ผิด | — | print error format แบบ bash: `minishell: syntax error near unexpected token '...'` |

**Data ที่ไหลผ่าน:** อ่าน `t_token *` อย่างเดียว ไม่สร้าง struct ใหม่ — ส่งต่อ **pointer เดิม** ไป Stage 3 ถ้าผ่าน หรือ return NULL ให้ main loop เคลียร์ list แล้ว reprompt ถ้าไม่ผ่าน

**ตัวอย่าง:**
```
input:  ls | | wc         → check_pipe_position() FAIL (pipe ติดกัน)
input:  cat <              → check_redir_has_target() FAIL (< ไม่มี target)
input:  echo hi | wc -l    → PASS ส่งต่อ Stage 3 ทั้ง list เดิม
```

---

### Stage 3 — EXPANSION (`parser_expand.c`)

**หน้าที่:** แทนที่ `$VAR` และ `$?` ด้วยค่าจริงจาก env / exit status **เฉพาะ token ที่ไม่ได้อยู่ใน single quote ทั้งหมด**

**Input:** `t_token *` (ผ่าน syntax check แล้ว) + `t_env *env` + `int last_exit_status`
**Output:** `t_token *` เดิม แต่ `value` ของแต่ละ token ถูกแทนที่ (mutate in place)

| ฟังก์ชัน | Input | Output | หน้าที่ |
|---|---|---|---|
| `void expand_all(t_token *head, t_env *env, int exit_status)` | token list + env + exit status | — (mutate) | วน loop เฉพาะ `TOKEN_WORD` — ถ้า `token->quote_kind == QK_SINGLE` **ข้ามไปเลยไม่ต้อง char-scan** (รู้แน่อยู่แล้วว่าไม่มี `$` ไหนต้อง expand) ตัวอื่น (`QK_NONE`/`QK_DOUBLE`/`QK_MIXED`) เรียก `expand_token_value()` ตามปกติ |
| `char *expand_token_value(char *val, t_env *env, int status)` | สตริงดิบของ 1 token | สตริงใหม่ (malloc) | scan ทีละตัวอักษร หา `$` ที่ไม่ได้อยู่ใน `'...'` แล้วแทนค่า |
| `char *extract_var_name(char *s, int *i)` | string + index หลัง `$` | ชื่อตัวแปร | ดึงชื่อ ตาม POSIX (`[A-Za-z_][A-Za-z0-9_]*`) หรือกรณีพิเศษ `?` |
| `char *get_var_value(char *name, t_env *env, int status)` | ชื่อตัวแปร | ค่า (หรือ `""` ถ้าไม่มี) | `?` → itoa(status), อื่นๆ → หาใน env list |

**Data ที่ไหลผ่าน:** `t_token->value` (char*) เข้า → เดิน scan → สร้าง buffer ใหม่ต่อกัน (คล้าย `ft_strjoin` วนหลายรอบ หรือ dynamic buffer เดียวเพื่อลด malloc) → free ของเก่า → เซ็ต `value` ใหม่

**สำคัญ:** ต้องเช็ค quote state **ระหว่าง scan** ไม่ใช่เช็คแค่ flag ระดับ token เพราะ 1 token อาจผสม `'no$expand'"yes$expand"` ได้ในคำเดียว → วิธีที่ใช้ได้จริง (ตาม mcombeau) คือเก็บ quote state ไล่ไปทีละตัวอักษรพร้อมกับ scan หา `$`

**ตัวอย่าง:**
```
env: USER=monkey
last_exit_status: 2

input token:  "hello $USER exit=$?"
              ↓ expand
output token: "hello monkey exit=2"

input token:  'hello $USER'     (single quote)
              ↓ expand (ไม่แตะ เพราะอยู่ใน single quote)
output token: 'hello $USER'     (เหมือนเดิมทุกตัวอักษร)

input token:  $NOTEXIST
              ↓ expand (ตัวแปรไม่มีค่า)
output token: ""   (string ว่าง — ไม่ใช่ NULL)
```

---

### Stage 4 — QUOTE REMOVAL (`parser_quote_strip.c`)

**หน้าที่:** ลบเครื่องหมาย `'` และ `"` ที่เหลือออกจากทุก token (ทำ**หลัง** expand เสมอ)

**Input:** `t_token *` (expand แล้ว)
**Output:** `t_token *` เดิม แต่ `value` ไม่มี quote character หลงเหลือ

**กฎบังคับ:** `strip_quotes()` ลบเฉพาะตัวอักษรใน `value` เท่านั้น **ห้ามแก้หรือ reset `token->quote_kind`** — Stage 5 ต้องอ่านค่านี้ต่อเพื่อรู้ว่า heredoc delimiter ถูก quote หรือไม่ (ดู 2.1) ถ้า Stage 4 ไป reset ค่านี้ (เช่นเข้าใจผิดว่าลบ quote แล้วต้อง "เคลียร์" flag ด้วย) Stage 5 จะไม่มีทางรู้ได้อีกเลย

| ฟังก์ชัน | Input | Output | หน้าที่ |
|---|---|---|---|
| `void strip_quotes_all(t_token *head)` | token list | — (mutate) | วน loop ทุก `TOKEN_WORD` |
| `char *strip_quotes(char *val)` | สตริงมี quote | สตริงไม่มี quote (malloc ใหม่) | scan ทีละตัวอักษร ข้าม `'` `"` ที่เป็นตัวเปิด/ปิด ไม่ข้ามตัวอักษรข้างใน |

**Data ที่ไหลผ่าน:** เหมือน Stage 3 คือ mutate `value` ใน token เดิม (free เก่า/set ใหม่)

**ตัวอย่าง:**
```
input:  "hello monkey"      → output: hello monkey
input:  'hello $USER'       → output: hello $USER   (ตัวอักษรเดิมเป๊ะ เพราะ stage 3 ไม่แตะ)
input:  hello"world"'!'     → output: helloworld!    (คำติดกันไม่มี space ต้อง join เป็นคำเดียว)
```

---

### Stage 5 — COMMAND BUILD (`parser_cmd_build.c`)

**หน้าที่:** แปลง `t_token *` (สะอาดแล้ว) ให้เป็น `t_command *` — จุดที่ pipeline เปลี่ยนจาก "list ของคำ" เป็น "list ของคำสั่งที่รันได้"

**Input:** `t_token *` (สะอาดแล้วจาก Stage 4)
**Output:** `t_command *` (list พร้อมส่งให้ Executor)

| ฟังก์ชัน | Input | Output | หน้าที่ |
|---|---|---|---|
| `t_command *build_commands(t_token *head)` | token list | command list | วน loop สร้าง `t_command` ใหม่ทุกครั้งที่เจอ `TOKEN_PIPE` หรือเริ่มต้น |
| `void collect_args(t_token **cur, t_command *cmd)` | pointer ไปยัง token ปัจจุบัน + cmd ที่กำลังสร้าง | — (mutate cmd->args) | เก็บ `TOKEN_WORD` ต่อเนื่องใส่ `args[]` จนกว่าจะเจอ operator |
| `t_redir *collect_redir(t_token **cur)` | pointer ไปยัง token ที่เป็น redir operator | `t_redir*` 1 node | อ่าน operator + WORD ถัดไป (ชื่อไฟล์/delimiter) สร้าง node แล้วขยับ pointer ไป 2 ตำแหน่ง — ถ้า operator เป็น `TOKEN_HEREDOC` อ่าน `delim_token->quote_kind` **ก่อน** เรียก `run_heredoc` (ค่านี้ยังอยู่ครบเพราะ Stage 4 ไม่แตะ — ดู 2.1) แล้วคำนวณ `heredoc_no_expand = (quote_kind != QK_NONE)` |
| `void append_redir(t_command *cmd, t_redir *new)` | cmd + redir node | — (mutate cmd->redirs) | ต่อท้าย list ของ redir (เพราะซ้อนได้) |
| `void mark_builtin(t_command *cmd)` | cmd ที่ args เต็มแล้ว | — (set is_builtin) | เทียบ `args[0]` กับ `echo/cd/pwd/export/unset/env/exit` |
| `t_heredoc_result run_heredoc(t_command *cmd, char *delim, bool no_expand)` | cmd + delimiter string + ค่า `heredoc_no_expand` ที่ `collect_redir` คำนวณไว้แล้วจาก `quote_kind` | enum `HEREDOC_OK` / `HEREDOC_INTERRUPTED` | **จุดเดียวที่ parser แตะ I/O จริง** — เรียก `readline()` วนจนกว่าจะเจอ delimiter, ถ้า `no_expand == true` เขียนแต่ละบรรทัดลงไฟล์ **โดยไม่ expand `$VAR`** เลย (ถ้า false ต้อง expand ปกติ) — ถ้าโดน `SIGINT` ระหว่างอ่าน ต้อง return `HEREDOC_INTERRUPTED` ทันที ไม่เขียนไฟล์ต่อ |

```c
// จุดที่แก้ blocker: quote_kind ต้องถูกอ่าน "ก่อน" Stage 4 จะลบตัวอักษรออกไม่ได้
// (Stage 4 รันเสร็จไปแล้วตั้งแต่ก่อน Stage 5 เริ่ม — แต่ field quote_kind ไม่ได้ถูกลบไปด้วย
// เพราะอยู่คนละที่กับ value) จึง "อ่านได้ปกติ" ที่ Stage 5 โดยไม่ต้องแหกกฎลำดับ stage ใดๆ
t_quote_kind qk = delim_token->quote_kind;
bool no_expand = (qk != QK_NONE);
t_heredoc_result r = run_heredoc(cmd, delim_token->value, no_expand);
if (r == HEREDOC_OK)
    redir->heredoc_no_expand = no_expand;
```

**Contract เรื่อง Ctrl-C ระหว่าง heredoc:** `run_heredoc()` return `HEREDOC_INTERRUPTED` → `build_commands()` ต้อง**หยุดสร้าง command ที่เหลือทั้งหมดทันที** และ return `NULL` ให้ main loop ทิ้งทั้ง `t_command` list ที่สร้างไปแล้วบางส่วน (free ทิ้ง ไม่ execute อะไรเลย) แล้ว reprompt ปกติ — เป็น contract เดียวกับ unclosed quote ใน Stage 1 (fail-fast ทิ้งทุกอย่างที่สร้างมาแล้ว)

**Data ที่ไหลผ่าน:**
```
t_token *cur  ──┐
                ├─ WORD, WORD         → args[] ของ t_command ปัจจุบัน
                ├─ REDIR_IN/OUT/APPEND + WORD ถัดไป          → t_redir node ต่อเข้า cmd->redirs
                ├─ HEREDOC + WORD ถัดไป (อ่าน quote_kind ที่ยังอยู่ครบ) → run_heredoc() → t_redir node (heredoc_no_expand ตั้งค่าแล้ว)
                └─ PIPE               → ปิด t_command ปัจจุบัน, สร้างอันใหม่ต่อท้าย list
```

**ตัวอย่างเดินเต็ม:**
```
Token (สะอาดแล้ว):
[0] WORD  cat
[1] REDIR_IN  <
[2] WORD  file.txt
[3] PIPE  |
[4] WORD  grep
[5] WORD  hello
[6] REDIR_OUT  >
[7] WORD  out.txt
[8] END

ผลลัพธ์ t_command list:

cmd1:
  args   = ["cat", NULL]
  redirs = [{type=REDIR_IN, file="file.txt", next=NULL}]
  is_builtin = false
  next -> cmd2

cmd2:
  args   = ["grep", "hello", NULL]
  redirs = [{type=REDIR_OUT, file="out.txt", next=NULL}]
  is_builtin = false
  next -> NULL
```

---

### Stage 6–8 — EXECUTOR (สรุปสั้น เพื่อ integration point)

> รายละเอียดเต็มเป็นของเพื่อน แต่พี่ต้องรู้ **contract** ที่ต้องส่งให้ตรง

| Stage | Input ที่ Executor ต้องการจาก Parser | หมายเหตุ integration |
|---|---|---|
| [6] PREP | `t_command *` ทั้ง list | ต้อง `next`/`prev` ครบเพื่อรู้ตำแหน่งหัว/ท้าย pipeline |
| [7] FORK/EXEC/BUILTIN | `cmd->args`, `cmd->redirs`, `cmd->is_builtin` | ถ้า `is_builtin == true` และ **ไม่มี pipe เลย** ให้รันใน process หลัก (ไม่ fork) — เกณฑ์นี้ mcombeau ใช้ และเป็นจุดที่ Executor ต้องเช็ค `cmd->prev == NULL && cmd->next == NULL` |
| [8] WAIT | — | คืนค่า `int exit_status` กลับมาที่ main loop เพื่อเก็บใน `g_exit_status` สำหรับ `$?` รอบถัดไป |

**Builtin function signature ที่ parser เตรียมให้ (แต่ implement โดยฝั่งไหนก็ได้ตาม pipeline):**
```c
int builtin_echo(char **args);
int builtin_cd(char **args, t_env **env);
int builtin_pwd(void);
int builtin_export(char **args, t_env **env);
int builtin_unset(char **args, t_env **env);
int builtin_env(t_env *env);
int builtin_exit(char **args, t_data *shell_state);
```
Builtin ใช้ `char **args` เดียวกับที่ Executor ใช้เรียก `execve()` — ไม่ต้องแปลง struct ซ้ำ ลดจุด bug ตรงกลาง

**หมายเหตุ `builtin_exit`:** `shell_state` ต้องพก pointer ไปยัง `t_command` ที่กำลังรันอยู่ด้วย (ไม่ใช่แค่ env/exit code) เพราะ `exit` ที่อยู่กลาง pipeline (`ls | exit | wc`) ต้อง exit เฉพาะ child process นั้น **ไม่** print `"exit"` ซ้ำ และไม่ปิด shell หลัก — ตัดสินจาก `cmd->prev != NULL || cmd->next != NULL` (มี pipe ล้อมอยู่ = quiet mode) ตรงข้ามกับ `exit` แบบยืนโดดเดี่ยวที่ต้อง print `"exit"` แล้วปิด shell จริง

---

## 5. ตัวอย่างเดินสมมติแบบเต็ม (Full Trace)

Input จาก user:
```
echo "hi $USER" | grep hi > out.txt
```
สมมติ `env: USER=monkey`, `last_exit_status = 0`

```
[readline]
  "echo \"hi $USER\" | grep hi > out.txt"

[Stage 1: LEXER]
  [0] WORD  echo
  [1] WORD  "hi $USER"
  [2] PIPE  |
  [3] WORD  grep
  [4] WORD  hi
  [5] REDIR_OUT  >
  [6] WORD  out.txt
  [7] END

[Stage 2: SYNTAX CHECK]
  ผ่าน (pipe ไม่ติดกัน, redir มี target) → ส่ง pointer เดิมต่อ

[Stage 3: EXPANSION]
  [1] WORD  "hi $USER"  →  "hi monkey"   (แทน $USER)
  (token อื่นไม่มี $ ไม่เปลี่ยน)

[Stage 4: QUOTE REMOVAL]
  [1] WORD  "hi monkey"  →  hi monkey    (ลบ quote)

[Stage 5: COMMAND BUILD]
  cmd1: args=["echo","hi monkey",NULL]  redirs=NULL       is_builtin=true
  cmd1.next -> cmd2
  cmd2: args=["grep","hi",NULL]         redirs=[{OUT,"out.txt"}]  is_builtin=false

[ส่งให้ Executor: t_command *cmd1]

[Stage 6-8: EXECUTOR]
  มี pipe → cmd1 ต้อง fork แม้เป็น builtin (เพราะ cmd->next != NULL)
  cmd1 (child): stdout → pipe write end, รัน builtin echo → พิมพ์ "hi monkey" เข้า pipe
  cmd2 (child): stdin ← pipe read end, stdout → out.txt, execve("grep", ["grep","hi"])
  parent: wait ทั้งสอง child, เก็บ exit status ของ "last command ใน pipeline" (cmd2) = $?

[ผลลัพธ์]
  out.txt มีเนื้อหา: "hi monkey"
  $?  = exit status ของ grep (0 ถ้าเจอ, 1 ถ้าไม่เจอ)
```

---

## 6. Checklist ครอบคลุม Mandatory Requirements

| Requirement (จาก subject) | Stage ที่รับผิดชอบ | ฟังก์ชันหลัก |
|---|---|---|
| Prompt แสดงผล + history | main loop (นอก pipeline) | `readline()` / `add_history()` |
| Quote `'` `"` | Lexer (scan), Quote Strip (ลบ), Expansion (เว้น single quote) | `lexer_scan_word`, `strip_quotes`, `expand_token_value` |
| Unclosed quote → error ไม่ crash | Lexer | `lexer_has_unclosed_quote`, `lexer()` return `NULL` |
| Redirection `<` `>` `>>` | Lexer, Command Build | `lexer_scan_operator`, `collect_redir` |
| Heredoc `<<` (รวม quoted delimiter ปิด expand) | Lexer (เซ็ต `quote_kind`), Command Build (อ่านค่า, ไม่คำนวณใหม่) | `run_heredoc` (+ `t_token.quote_kind` → `t_redir.heredoc_no_expand`) |
| Pipe `\|` | Lexer, Syntax Check, Command Build, Executor | `lexer_scan_operator`, `check_pipe_position`, `build_commands` |
| Env var `$VAR` | Expansion | `expand_token_value`, `get_var_value` |
| `$?` | Expansion | `extract_var_name` (กรณีพิเศษ `?`) |
| Builtin ทั้ง 7 ตัว | Command Build (mark) + Executor (เรียกจริง) | `mark_builtin` + `builtin_*` |
| Signal (ctrl-C/D/\\) นอก heredoc | นอก pipeline (main loop + Executor) | ไม่อยู่ใน scope เอกสารนี้ |
| Signal (ctrl-C) **ระหว่าง heredoc** | Command Build (จุดเดียวที่ parser ต้องรับ SIGINT) | `run_heredoc` return `HEREDOC_INTERRUPTED` → `build_commands` abort ทั้ง list |
| Exit code ส่งต่อ (`$?`) | Executor → main loop → Expansion รอบถัดไป | `g_exit_status` |

---

## 7. สิ่งที่ต้องระวังเป็นพิเศษ (จาก bug ที่เจอมาแล้ว + reference repos)

1. **Lexer ต้อง scan ทีละตัวอักษร** — เคยลองใช้ `ft_split` แล้วผิด เพราะ space ใน quote ไม่ควรตัดคำ
2. **malloc token ใหม่ต้อง init field ให้ครบ** ทุกครั้ง (`prev=NULL, next=NULL, quote_kind=QK_NONE` ฯลฯ) — bug เดิมคือ malloc init ผิดทำให้ list พังตอน free
3. **ห้าม strdup ซ้ำสองรอบ** โดยไม่ได้ตั้งใจ — เช็คทุกจุดที่สร้าง token ว่า duplicate string แค่ครั้งเดียว
4. **Stage 3 ต้องมาก่อน Stage 4 เสมอ** — ไม่งั้นแยกไม่ออกว่าตรงไหนเคยอยู่ใน single quote
5. **Builtin ตัดสินใจที่ Command Build (mark) แต่รันจริงที่ Executor** — เพราะ criterion คือ side effect ไม่ใช่ fork/execve (ตามที่ตกลงกันไว้แล้ว)
6. **redirs ต้องเป็น list ไม่ใช่ struct เดี่ยว** — กัน `cat < a < b > c` ทิ้งข้อมูลโดยไม่ได้ตั้งใจ
7. **Variable ไม่มีค่า → string ว่าง ไม่ใช่ NULL** — ถ้าเป็น NULL จะ crash ตอน strlen/strcmp ที่ stage หลัง
8. **Heredoc เป็นจุดเดียวที่ parser แตะ I/O จริง** — ต้องแยกออกจาก parser ที่เหลือให้ชัด (คนละไฟล์ `parser_heredoc.c`) เพื่อไม่ให้ side effect ปนกับ stage อื่น
9. **Unclosed quote ต้อง fail-fast ที่ Lexer** — `echo "hello` (ไม่มี quote ปิด) ต้อง print `unclosed quotes` แล้ว return `NULL` ทันที ไม่ปล่อยให้ Stage 2-5 เห็น token ที่ผิดรูป
10. **Heredoc delimiter ที่ถูก quote (`<< "EOF"` / `<< 'EOF'`) ต้องปิด `$VAR` expansion ทั้ง body** — ต่างจาก `<< EOF` ธรรมดาที่ต้อง expand ปกติ **จุดสำคัญ:** ข้อมูล "ถูก quote หรือไม่" มาจาก `token->quote_kind` ที่ Lexer เซ็ตไว้ตั้งแต่ Stage 1 — **Stage 4 (Quote Removal) ห้ามลบ field นี้** แม้จะลบตัวอักษร quote ออกจาก `value` แล้ว เพราะ Stage 5 ต้องอ่านค่านี้ต่อเพื่อคำนวณ `t_redir.heredoc_no_expand` (ถ้าลบทิ้งไปพร้อมกัน Stage 5 จะไม่มีทางรู้ได้อีกเลย)
11. **Ctrl-C ระหว่างรอ heredoc input ต้อง abort ทั้ง command list** — ไม่ execute อะไรเลยแม้จะ parse ผ่านไปแล้วบางส่วน (เหมือน bash จริง)

---

*เอกสารนี้ครอบคลุมเฉพาะ mandatory part ตาม 42 subject — ไม่รวม bonus (`&&`, `||`, `()`, wildcard `*`)*
