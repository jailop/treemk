# Code Blocks

TreeMk provides powerful syntax highlighting for code blocks, making it easy to document programming code and technical content.

## Basic Code Blocks

Create a code block by wrapping your code with triple backticks:

````
```
def hello():
    print("Hello, World!")
```
````

## Syntax Highlighting

Specify the programming language after the opening backticks for syntax highlighting:

````
```python
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n-1) + fibonacci(n-2)

# Calculate the 10th Fibonacci number
result = fibonacci(10)
print(f"Fibonacci(10) = {result}")
```
````

## Supported Languages

TreeMk supports syntax highlighting for many languages:

### Python
````
```python
import numpy as np

def matrix_multiply(A, B):
    return np.dot(A, B)

A = np.array([[1, 2], [3, 4]])
B = np.array([[5, 6], [7, 8]])
print(matrix_multiply(A, B))
```
````

### JavaScript
````
```javascript
const fetchData = async (url) => {
    try {
        const response = await fetch(url);
        const data = await response.json();
        return data;
    } catch (error) {
        console.error('Error:', error);
    }
};

fetchData('https://api.example.com/data')
    .then(data => console.log(data));
```
````

### C++
````
```cpp
#include <iostream>
#include <vector>

template<typename T>
class Stack {
private:
    std::vector<T> elements;
public:
    void push(T const& elem) {
        elements.push_back(elem);
    }
    T pop() {
        T elem = elements.back();
        elements.pop_back();
        return elem;
    }
};

int main() {
    Stack<int> intStack;
    intStack.push(42);
    std::cout << intStack.pop() << std::endl;
    return 0;
}
```
````

### Java
````
```java
public class BinarySearch {
    public static int search(int[] arr, int target) {
        int left = 0, right = arr.length - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            
            if (arr[mid] == target)
                return mid;
            if (arr[mid] < target)
                left = mid + 1;
            else
                right = mid - 1;
        }
        return -1;
    }
}
```
````

### Rust
````
```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5];
    
    let sum: i32 = numbers.iter().sum();
    let doubled: Vec<i32> = numbers.iter()
        .map(|x| x * 2)
        .collect();
    
    println!("Sum: {}", sum);
    println!("Doubled: {:?}", doubled);
}
```
````

### SQL
````
```sql
SELECT u.name, COUNT(o.id) as order_count
FROM users u
LEFT JOIN orders o ON u.id = o.user_id
WHERE o.created_at > '2024-01-01'
GROUP BY u.name
HAVING COUNT(o.id) > 5
ORDER BY order_count DESC;
```
````

### Shell/Bash
````
```bash
#!/bin/bash

# Backup script
BACKUP_DIR="/backup"
SOURCE_DIR="/home/user/documents"
DATE=$(date +%Y%m%d)

tar -czf "$BACKUP_DIR/backup_$DATE.tar.gz" "$SOURCE_DIR"

if [ $? -eq 0 ]; then
    echo "Backup completed successfully"
else
    echo "Backup failed"
    exit 1
fi
```
````

### HTML/CSS
````
```html
<!DOCTYPE html>
<html>
<head>
    <style>
        .container {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }
        .card {
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            border-radius: 8px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="card">Hello, World!</div>
    </div>
</body>
</html>
```
````

### JSON
````
```json
{
    "name": "TreeMk",
    "version": "1.0.0",
    "features": [
        "Markdown editing",
        "Wiki links",
        "Code highlighting",
        "LaTeX formulas"
    ],
    "config": {
        "theme": "dark",
        "autoSave": true
    }
}
```
````

### YAML
````
```yaml
version: '3.8'
services:
  web:
    image: nginx:latest
    ports:
      - "80:80"
    volumes:
      - ./html:/usr/share/nginx/html
  database:
    image: postgres:14
    environment:
      POSTGRES_PASSWORD: secret
      POSTGRES_DB: myapp
```
````

## Inline Code

For inline code, use single backticks:

```
Use the `print()` function to output text in Python.
```

Renders as: Use the `print()` function to output text in Python.

## Tips

- **Language name** - Use lowercase language names after backticks
- **Common names** - `js` works for JavaScript, `py` for Python, `cpp` for C++
- **Plain text** - Omit language name for unformatted code blocks
- **Line wrapping** - Long lines will wrap in the preview
- **Copy-paste** - Code blocks preserve formatting when copied

## Supported Language Names

Here are some commonly used language identifiers:

- `python`, `py` - Python
- `javascript`, `js` - JavaScript
- `typescript`, `ts` - TypeScript
- `java` - Java
- `c`, `cpp`, `c++` - C/C++
- `csharp`, `cs` - C#
- `rust`, `rs` - Rust
- `go` - Go
- `ruby`, `rb` - Ruby
- `php` - PHP
- `swift` - Swift
- `kotlin`, `kt` - Kotlin
- `scala` - Scala
- `shell`, `bash`, `sh` - Shell scripts
- `sql` - SQL
- `html` - HTML
- `css` - CSS
- `json` - JSON
- `xml` - XML
- `yaml`, `yml` - YAML
- `markdown`, `md` - Markdown
- `diff` - Diff/Patch

## Export

When exporting your notes, code blocks are preserved with syntax highlighting in:
- **HTML export** - Fully styled code blocks
- **PDF export** - Code with highlighting via Pandoc
- **DOCX export** - Code blocks formatted as monospace text
