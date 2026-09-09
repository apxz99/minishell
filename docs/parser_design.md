```mermaid
---
config:
  layout: elk
---
flowchart TD

    %% =========================================================
    %% INPUT
    %% =========================================================

    A["INPUT<br/><br/>User พิมพ์:<br/>echo &quot;hello $USER&quot; &gt; out.txt<br/><br/>Data: char *input"]


    %% =========================================================
    %% STAGE 1 — LEXER
    %% =========================================================

    subgraph S1["STAGE 1 — LEXER"]
        direction TB

        B["อ่าน input ทีละ character<br/>แยก WORD / PIPE / REDIRECTION<br/>จัดการ quote state"]

        C{"Character เป็นอะไร?"}

        D["space<br/>ข้าม whitespace<br/>ไม่สร้าง Token"]

        E["letter / normal char<br/>อ่านต่อเป็น WORD"]

        F{"ตัวถัดไปเหมือนกันไหม?"}

        G["quote<br/>เข้า Quote State<br/>SINGLE_QUOTE / DOUBLE_QUOTE<br/>เก็บ quote ไว้ก่อน"]

        H{"เป็น &lt; หรือ &gt; ?"}

        I["สร้าง Operator 2 ตัว<br/><br/>&lt;&lt; = HEREDOC<br/>&gt;&gt; = APPEND"]

        J["สร้าง Operator 1 ตัว<br/><br/>&lt; = REDIR_IN<br/>&gt; = REDIR_OUT"]

        K{"ยังมี character?"}

        L["สร้าง TOKEN_END"]

        M["LEXER RESULT<br/><br/>t_token *<br/><br/>WORD: echo<br/>WORD: &quot;hello $USER&quot;<br/>REDIR_OUT: &gt;<br/>WORD: out.txt<br/>END"]

        B --> C
        C -->|space| D
        C -->|letter / normal char| E
        C -->|&lt; หรือ &gt;| H
        C -->|quote| G

        H -->|YES| F
        H -->|NO| J

        F -->|YES| I
        F -->|NO| J

        D --> K
        E --> K
        G --> K
        I --> K
        J --> K

        K -->|YES| C
        K -->|NO| L
        L --> M
    end


    %% =========================================================
    %% STAGE 2 — SYNTAX
    %% =========================================================

    subgraph S2["STAGE 2 — SYNTAX CHECK"]
        direction TB

        N["ตรวจ Token ว่าเรียงถูกต้องหรือไม่<br/>ไม่ Expand / ไม่ Execute"]

        O{"Token type?"}

        P["WORD<br/>ถูกต้อง<br/>ตรวจ Token ถัดไป"]

        Q{"มี Command<br/>ก่อนและหลัง PIPE?"}

        R{"มี WORD<br/>เป็น target หลัง redirection?"}

        T{"ตรวจ Token ครบแล้ว?"}

        U["SYNTAX OK<br/><br/>validated t_token *"]

        V["SYNTAX ERROR<br/><br/>ตัวอย่าง:<br/>| echo<br/>echo |<br/>echo &gt;"]

        N --> O

        O -->|WORD| P
        O -->|PIPE| Q
        O -->|REDIRECTION| R

        Q -->|YES| P
        Q -->|NO| V

        R -->|YES| P
        R -->|NO| V

        P --> T

        T -->|NO| O
        T -->|YES| U
    end


    %% =========================================================
    %% STAGE 3 — EXPANSION
    %% =========================================================

    subgraph S3["STAGE 3 — EXPANSION + QUOTE REMOVAL"]
        direction TB

        W["อ่าน WORD<br/>ทีละ character"]

        X{"อยู่ใน SINGLE_QUOTE?"}

        Y["เก็บเป็น Literal<br/><br/>$USER → $USER"]

        Z{"เจอ $ ไหม?"}

        AA["เก็บ character เดิม"]

        AB{"เป็น $? หรือไม่?"}

        AC["ใช้ last_exit_status<br/><br/>$? → 0"]

        AD["extract_var_name()"]

        AE{"Variable name valid?"}

        AF["$ เป็น Literal"]

        AG["ค้น Variable จาก env"]

        AH{"พบ Variable ไหม?"}

        AI["append value<br/><br/>USER → airport"]

        AJ["ใช้ Empty String<br/><br/>$UNKNOWN → &quot;&quot;"]

        AK["append result"]

        AL{"ยังมี character?"}

        AM["QUOTE REMOVAL<br/><br/>ลบ ' และ &quot; ที่ใช้ควบคุม shell<br/><br/>&quot;hello airport&quot;<br/>↓<br/>hello airport"]

        AN["EXPANSION RESULT<br/><br/>t_token *<br/><br/>WORD: echo<br/>WORD: hello airport<br/>REDIR_OUT: &gt;<br/>WORD: out.txt"]

        W --> X

        X -->|YES| Y
        X -->|NO| Z

        Z -->|NO| AA
        Z -->|YES| AB

        AB -->|YES| AC
        AB -->|NO| AD

        AD --> AE

        AE -->|NO| AF
        AE -->|YES| AG

        AG --> AH

        AH -->|YES| AI
        AH -->|NO| AJ

        Y --> AK
        AA --> AK
        AC --> AK
        AF --> AK
        AI --> AK
        AJ --> AK

        AK --> AL

        AL -->|YES| W
        AL -->|NO| AM
        AM --> AN
    end


    %% =========================================================
    %% STAGE 4 — COMMAND BUILDER
    %% =========================================================

    subgraph S4["STAGE 4 — COMMAND BUILDER"]
        direction TB

        AO["อ่าน Token ทีละตัว<br/>จัดกลุ่มเป็น Command"]

        AP{"Token type?"}

        AQ["WORD<br/><br/>เพิ่ม value เข้า argv<br/><br/>echo → argv[0]<br/>hello airport → argv[1]"]

        AR["REDIRECTION<br/><br/>สร้าง t_redir<br/>เก็บ type + target<br/><br/>&gt; + out.txt"]

        AS["PIPE<br/><br/>จบ t_cmd ปัจจุบัน<br/>สร้าง t_cmd ตัวถัดไป"]

        AT["t_cmd<br/><br/>argv + redirection"]

        AU{"ยังมี Token?"}

        AV["BUILDER RESULT<br/><br/>t_cmd *<br/><br/>argv:<br/>[&quot;echo&quot;, &quot;hello airport&quot;]<br/><br/>redir:<br/>&gt; out.txt"]

        AO --> AP

        AP -->|WORD| AQ
        AP -->|REDIRECTION| AR
        AP -->|PIPE| AS

        AQ --> AT
        AR --> AT
        AS --> AT

        AT --> AU

        AU -->|YES| AP
        AU -->|NO| AV
    end


    %% =========================================================
    %% STAGE 5 — EXECUTOR
    %% =========================================================

    subgraph S5["STAGE 5 — EXECUTOR"]
        direction TB

        AW["รับ t_cmd *<br/>Execute จริง"]

        AX{"มี Pipeline?"}

        AY["สร้าง pipe()<br/>เชื่อม stdin / stdout"]

        AZ["Single Command"]

        BA["fork()"]

        BB["Setup Redirection<br/><br/>open()<br/>dup2()<br/>close()"]

        BC{"เป็น Builtin?"}

        BD{"อยู่ใน Pipeline?"}

        BE["Run Builtin ใน Parent<br/><br/>cd /tmp<br/>export X=1<br/>exit"]

        BF["Run Builtin ใน Child<br/><br/>cd /tmp | cat"]

        BG["execve()<br/><br/>External Command"]

        BH["wait / collect status"]

        BI["Update last_exit_status<br/><br/>ค่าที่ใช้โดย $?"]

        BJ["กลับ Main Loop"]

        AW --> AX

        AX -->|YES| AY
        AX -->|NO| AZ

        AY --> BA
        AZ --> BA

        BA --> BB
        BB --> BC

        BC -->|YES| BD
        BC -->|NO| BG

        BD -->|NO| BE
        BD -->|YES| BF

        BG --> BH
        BE --> BH
        BF --> BH

        BH --> BI
        BI --> BJ
    end


    %% =========================================================
    %% HEREDOC SPECIAL CASE
    %% =========================================================

    subgraph SH["HEREDOC — SPECIAL REDIRECTION"]
        direction TB

        BK["TOKEN_HEREDOC<br/><br/>&lt;&lt; delimiter"]

        BL["อ่าน WORD ถัดไป<br/>เป็น delimiter"]

        BM{"Delimiter มี Quote?"}

        BN["Body Expansion = ON<br/><br/>$USER → airport"]

        BO["Body Expansion = OFF<br/><br/>&lt;&lt; 'EOF'<br/>$USER → $USER"]

        BP["อ่าน input จนพบ delimiter"]

        BQ["ส่งข้อมูลเข้า pipe / temp fd<br/>ให้ command ใช้เป็น stdin"]

        BK --> BL
        BL --> BM

        BM -->|NO| BN
        BM -->|YES| BO

        BN --> BP
        BO --> BP

        BP --> BQ
    end


    %% =========================================================
    %% PIPE DATA
    %% =========================================================

    subgraph SP["PIPE DATA STRUCTURE"]
        direction LR

        BR["t_cmd #1<br/><br/>argv = [&quot;cat&quot;, &quot;file.txt&quot;]"]

        BS["t_cmd #2<br/><br/>argv = [&quot;grep&quot;, &quot;hello&quot;]"]

        BT["t_cmd #3<br/><br/>argv = [&quot;wc&quot;, &quot;-l&quot;]"]

        BR --> BS --> BT
    end


    %% =========================================================
    %% DATA TRANSFORMATION
    %% =========================================================

    subgraph SD["DATA TRANSFORMATION"]
        direction LR

        BU["char *input"]

        BV["t_token *"]

        BW["validated t_token *"]

        BX["expanded t_token *"]

        BY["t_cmd *"]

        BZ["Process<br/>exit status"]

        BU -->|Lexer| BV
        BV -->|Syntax| BW
        BW -->|Expansion| BX
        BX -->|Builder| BY
        BY -->|Executor| BZ
    end


    %% =========================================================
    %% MAIN PIPELINE CONNECTION
    %% =========================================================

    A --> S1
    S1 --> S2
    S2 -->|SYNTAX OK| S3
    S2 -->|SYNTAX ERROR| BJ
    S3 --> S4
    S4 --> S5
    S5 --> BJ


    %% =========================================================
    %% SPECIAL CONNECTIONS
    %% =========================================================

    S4 -.->|พบ &lt;&lt;| SH
    SH -.->|stdin ของ command| S5

    S4 -.->|PIPE| SP
    SP -.->|execute pipeline| S5

    S1 -.-> SD
    S2 -.-> SD
    S3 -.-> SD
    S4 -.-> SD
    S5 -.-> SD
