# CBQN system commands

These are commands usable from a CBQN REPL that, for one reason or another, aren't suited to be system functions.

## `)ex path/to/file`

Execute the contents of the file as if it were REPL input (but allowing multiline definitions). Not a system function because modifying the list of global variables during execution is not allowed.

## `)t expr` / `)time expr` / `)t:n expr` / `)time:n expr`

Time the argument expression. `n` specifies the number of times to repeat. Exists to allow not escaping quotes and less overhead for timing very fast & small expressions.

## `)mem`

Get statistics on memory usage.

`)mem t` to get usage per object type.  
`)mem s` to get a breakdown of the number of objects with a specific size.  
`)mem f` to get breakdown of free bucket counts per size.

## `)erase name`

Erase the specified variable name.

Not a system function because it only clears the variables value (previous code `↩`ing it will still be able to), and to allow it to be executed even when the VM is completely out of memory such that it can't even parse an expression.

## `)vars`

List the globally defined variables.

## `)gc`

Force garbage collection.

`)gc disable` disables automatic garabage collection, and `)gc enable` enables it again.

Not a system function because currently CBQN doesn't support garbage collection in the middle of program execution.

## `)internalPrint expr`

Use the internal object printing system to show the expression result.