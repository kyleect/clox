# Language Goals (V1)

## Collections

### Current

There are no arrays, lists, tuples, or other collection type.

### Ideal

| Type  | Size    | Types      |
| ----- | ------- | ---------- |
| Array | Fixed   | Homogenous |
| List  | Dynamic | Mixed      |
| Tuple | Fixed   | Mixed      |

## Iterators

- Strings
- Arrays
- Lists
- Tuples

## Character Type

### Current

There is no specific "character" type, only string.

### Ideal

The language plans on supporting characters using single quotes `'a', 'b'`

## Different Number Types

### Current

Lox only has one kind of number type: `double`.

### Ideal

The language plans support for diffent kinds of numbers.

- `u8|16|32|64` unsigned integers
- `i8|16|32|64` signed integers
- `f32|64` floating point numbers

## No Top Level Side Effects

### Current

In lox there are only "scripts" which allow both top level declarations but also statements.

### Ideal

The language plans on supporting modules and to avoid side effects from importings e.g. javascript, the language will only allow top level declarations.

#### Exception

It's likely there will be three kinds of language files: scripts, modules, binaries. Scripts are how lox files behave now. They would allow for top level side effects but they can't be imported as modules.

## Function Declarations

### Current

```lox
fun sum (a, b) {
    return a + b;
}
```

### Ideal

```
fn sum (a, b) {
    a + b
}

fn sum(a, b) = a + b;

let sum = (a, b) => a + b;

fn apply(value, f) = f(value);

let result = apply(10, (value) => value * value);
```

## Variable Declarations

### Current

```lox
var greeting = "Hello";
var name;

name = "World";
```

### Ideal

Using the `let` keyword to declare variables. The `mut` keyword denotes the variable can be mutated.

```
let greeting = "Hello";
let mut name;

name = "World";
```

## Structs Not Classes

- Public fields/methods by default
- Dynamically add and remove fields/methods
- Implicit `this` in scope
- Methods declared in class declaration block

### Current

```
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }

    copy(person) {
        this.name = person.name;
        this.age = person.age;
    }
}

var person = Person("John", 30);
```

### Ideal

- Private fields/methods by default
- No dynamically adding/removing fields/methods
- Default initializer values
- Passing instance (`this`) to methods
- Static methods don't accept `this` arg
- Implementation `impl` blocks
- No default constructor

```
struct Person {
    name = "Unnamed";
    age;
}

impl Person {
    fn new(name, age) {
        return Person {
            name = "John",
            age  = 30
        };
    }

    fn copy(this, person) {
        this.name = person.name;
        this.age = person.age;
    }
}

let person = Person::new("John", "30);
```

## Implicit Returns

## Expression Blocks
