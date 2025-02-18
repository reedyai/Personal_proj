smallsh - A Minimal Shell Implementation

**Command Syntax**

    Commands follow this general structure:

    ": command [arg1 arg2 ...] [< input_file] [> output_file] [&]"

    Commands are words separated by spaces.

    <, >, and & must be surrounded by spaces.

    & at the end runs the command in the background.

    Input (<) and output (>) redirection must appear after all arguments.

    No support for quoted arguments or the | operator.

    Max command length: 2048 characters; Max arguments: 512.

    No syntax error checking.

**Comments & Blank Lines**

    Lines starting with # are ignored.

    Blank lines do nothing; the shell re-prompts.

**Variable Expansion**

    $$ expands to the shell's process ID.

    No other variable expansion is performed.

**Built-in Commands**

    exit: Terminates the shell, killing any child processes.

    cd: Changes the working directory (default: $HOME).

    status: Prints the exit status or terminating signal of the last foreground process.

    Built-in commands always run in the foreground and do not support I/O redirection.

**Executing Other Commands**

    Non-built-in commands are executed via fork(), exec(), and waitpid().

    Uses $PATH to locate executables.

    If a command is not found, prints an error and sets exit status to 1.

**Input & Output Redirection**

    Uses dup2() to handle < input_file and > output_file.

    Input redirection fails if the file cannot be read.

    Output redirection fails if the file cannot be written.

    Both can be used simultaneously.

**Foreground & Background Execution**

    Foreground: No & at the end; the shell waits for completion.

    Background: Ends with &; shell does not wait and prints the process ID.

    If input/output is not redirected for a background process, it is redirected to /dev/null.

**Handling Signals**

    SIGINT (CTRL-C):

        Shell ignores it.

        Foreground child processes terminate on receipt.

        If a foreground process is terminated, the shell prints the signal number.

    SIGTSTP (CTRL-Z):

        Toggles background execution.

        When disabled, & is ignored, running all commands in the foreground.

        Another CTRL-Z re-enables background execution.

**Example Usage**

: ls -l > output.txt
: sort < input.txt > sorted.txt &
: # This is a comment
: cd /home/user
: status

