![bass](doc/bass.svg)


# Plek
Rewrite of nears Bass Assembler

## Goal
Plek is an new macro frontend for Nears Table Assembly Engine. I started it having the wish to build a foundation for futher features. The current framework even struggles to have spaces on 'bad places' and to be honest, I found no way of solving this, but to switch from 'replace stuff' to 'parse stuff'. In other words: Redo the whole frontend from scratch. 

## Syntax

### Lazy writings
I added tons of short ways to write to pepple our users lazyness.

- `constant` can also be `const`
- `variable` can also be `var`
- `macro` or `function` can also be `fun` - yes, no hamburgers.
- `architecture` can also be `arch`

Most of them had been around allready. Now they are back.

### Everything is a function
Bass had it's internal reason to mix command's and build-in-functions. This had been removed. Everything is a function now.
```
print "Hello World\n"  // not working
print("Hello World\n") // working
```

### Dataypes
Since I upgraded bass from an macro to an script language it also knows some Datatypes. They will be selected implicite by the value that you store inside of it.

* Integer - internally stored as int64 
* Float - internally stored as int64 
* String - internally stored as, well, string
* function - stored as hashmap given by its name and signature
* array - stored as hashmap given by the used key


### Include
Include had several special rules. This had been changed. You can include your stuff where and whenever you want. Even with constructed file-names. Note that an include file will be loaded and parsed just once.

### Namespaces 
The namespace feature remains nearby unchanged to the given syntax in bass18:

```
namespace foo.baa {
  fun test() { print "Hello World\n" }
}

foo.baa.test() // works
```
However. Bass always had an different namespace for different things (var,const,macros,namespaces,arrays, etc..) - this is **NOT** the case anymore. Everything shares one namespace now.

### Assembly
Assembly code may use alot of stuff that looks like script syntax in the first place. Like some commands include dots. Others a wide number of braches. To avoid strange errors it is good to know how bass notices 'hey, this is assembly and not macro syntax'. It will follow this rules:

- It is the beginning of a new line, or an new statement
- This is no declaration, no label, no assignment, no namespace, and also not a command or anything else that 'starts' a line.

Then it must be assembly! 

Bass will try to solve the parameters for you. 

* Identifiers and function calls should work out of the box. But **avoid** using parameters that have the same name as parts of your fixed assembly syntax.
* Inline math an other stuff is not possible. Since assembly commands may use parentheses and brackets (`() and []`) by themselfes you need to use the 'manual identifyer'-syntax if you really need to do stuff inline.

```
lda.b MY_CONST_VALUE
lda.b getConfig()
lda.b {(100+12)}
```
This will work. 


### Macros / Functions
> There are no Macro's. There is only zuul

Macros had been removed and replaced by functions. While using them is nearby identical `foo()` compared to before, they are not just copy and paste anymore. Each function 

-  gets invoked
-  has his own scope
-  will crawl up on the scope tree whenever a symbol is missed and cannot be resolved
-  has an return value

```
var hello = "Hello "
fun greet(name) {
  return hello + name + "\n"
}

print(greet("World"))  // prints 'Hello World\n'
```

In other words: It should behave like you would expect it from a function. Ah and yes, overloading is still possible.

### Arrays
'Arrays' are hash-maps. They can be created using the construction function `var myarray = Array.new(1, ..)`. This syntax had been choosen to be as close as possible to the given array functions `Array.size()` and `Array.sort()`. On const arrays everything inside is constant. So its not possible to add, change, remove or sort its content.  Access is possible with the common `myarray[index]` syntax. Arrays to not have a fixed size. All array-keys will be thread as strings, other types will be converted.

### Special Syntax
One of bass strong points had always been, that it was basically just replacing stuff. This is not anymore. And I did my best to add some features that ensure, that you still can do ALOT more then usual script languages would allow you, to implement cool features all by yourself.

Note one thing: Special syntax always allows only one parameter. If you need more, you will need to store your content in a variable, or simply enclose it by and parentheses like `("more"+"here")`


#### Access brackets
The general meaning of brackets `[ ]` is to access something by an given index or name. This is generally used in Array's, but might be used without identifier as well.
```
var foo = 42
var baa = "foo"
var tmp = [baa]
print(["foo"], tmp)  // prints 4242
```

#### manual identifyer
They allow you to store something by an just created name. Just surround it by braces `{ }`
```
var i = 1
var {"name"} = 12
const {"solution"} = 42
{ ("labelNo"+i) }:
```
