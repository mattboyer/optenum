#optenum

optenum(1) is a command-line option enumerator for ELF executables.

![Illustrative video](/doc/optenum.gif?raw=true)

optenum(1) uses static analysis to extract the options accepted by a binary and lets you use [bash](http://www.gnu.org/software/bash/)'s autocompletion with options.

`<space> <dash> <TAB> <TAB>` - it's *magic*!

optenum(1) extracts options from binary executables by relying on the assumption that the task of defining valid command line options is done through a call to one of the following supported functions:

<table>
<thead><tr><th>Function</th><th>Library</th><th>Used by</th></tr></thead>
<tbody>
<tr><td><tt>getopt</tt></td><td rowspan="3">libc</td><td rowspan="3">GNU Coreutils, systemd utils, GNU NetCat and many more POSIX-compliant tools</td></tr>
<tr><td><tt>getopt_long</tt></td></tr>
<tr><td><tt>getopt_long_only</tt></td></tr>
<tr><td><tt>g_option_context_add_main_entries</tt></td><td>Glib</td><td>Gimp, Xchat,...</td></tr>
</tbody>
</table>

optenum(1) uses `libbfd` from the [GNU binutils](http://www.gnu.org/software/binutils/) to parse the dynamic symbols used by a binary executable and disassemble its code.

When it finds a call to one of the supported option-parsing functions above, optenum will attempt to reconstitute the arguments passed as part of the call and, in the even the argument(s) that describes valid options has successfully been retrieved and points to a chunk of data hardcoded in the binary, finally parses it and exposes options to the user.

optenum(1) **never** executes foreign code and doesn't rely on any particular behaviour in the target binary. No usage message? No problem!

There are several moving parts and optenum operates on a best-effort basis. When optenum can't retrieve options, it will try to fail gracefully and fail fast.

optenum(1) only supports `x86_64` argument passing conventions at this time. Support for 32-bit `i386` binaries is planned with other architectures a possibility.
