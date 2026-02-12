# Mathematical Formulas

TreeMk supports rendering mathematical formulas using LaTeX syntax powered by KaTeX. You can write both inline and display (block) formulas.

## Inline Formulas

For inline formulas within text, wrap your LaTeX code with single dollar signs `$...$`:

**Example:**
```
The quadratic formula is $x = \frac{-b \pm \sqrt{b^2-4ac}}{2a}$ where $a \neq 0$.
```

**Renders as:**  
The quadratic formula is $x = \frac{-b \pm \sqrt{b^2-4ac}}{2a}$ where $a \neq 0$.

## Display Formulas

For display formulas that appear on their own line, use double dollar signs `$$...$$`:

**Example:**
```
$$
E = mc^2
$$
```

**Renders as:**

$$
E = mc^2
$$

## Common Mathematical Notation

### Fractions and Roots

```
$$
\frac{1}{2} \quad \frac{a}{b+c} \quad \sqrt{x} \quad \sqrt[3]{x}
$$
```

$$
\frac{1}{2} \quad \frac{a}{b+c} \quad \sqrt{x} \quad \sqrt[3]{x}
$$

### Superscripts and Subscripts

```
$$
x^2 \quad x^{10} \quad x_i \quad x_{ij} \quad x_i^2
$$
```

$$
x^2 \quad x^{10} \quad x_i \quad x_{ij} \quad x_i^2
$$

### Greek Letters

```
$$
\alpha, \beta, \gamma, \Delta, \Theta, \pi, \omega, \Omega
$$
```

$$
\alpha, \beta, \gamma, \Delta, \Theta, \pi, \omega, \Omega
$$

### Summation and Integration

```
$$
\sum_{i=1}^{n} x_i \quad \int_{0}^{\infty} e^{-x} dx
$$
```

$$
\sum_{i=1}^{n} x_i \quad \int_{0}^{\infty} e^{-x} dx
$$

### Matrices

```
$$
\begin{pmatrix}
a & b \\
c & d
\end{pmatrix}
\quad
\begin{bmatrix}
1 & 2 & 3 \\
4 & 5 & 6
\end{bmatrix}
$$
```

$$
\begin{pmatrix}
a & b \\
c & d
\end{pmatrix}
\quad
\begin{bmatrix}
1 & 2 & 3 \\
4 & 5 & 6
\end{bmatrix}
$$

## Advanced Examples

### Calculus

The derivative of a function:

```
$$
f'(x) = \lim_{h \to 0} \frac{f(x+h) - f(x)}{h}
$$
```

$$
f'(x) = \lim_{h \to 0} \frac{f(x+h) - f(x)}{h}
$$

### Linear Algebra

System of equations:

```
$$
\begin{cases}
2x + 3y = 5 \\
x - y = 1
\end{cases}
$$
```

$$
\begin{cases}
2x + 3y = 5 \\
x - y = 1
\end{cases}
$$

### Statistics

Normal distribution probability density function:

```
$$
f(x) = \frac{1}{\sigma\sqrt{2\pi}} e^{-\frac{1}{2}\left(\frac{x-\mu}{\sigma}\right)^2}
$$
```

$$
f(x) = \frac{1}{\sigma\sqrt{2\pi}} e^{-\frac{1}{2}\left(\frac{x-\mu}{\sigma}\right)^2}
$$

## Tips

- **Preview** - Use the preview pane to see your formulas rendered in real-time
- **Escaping** - To show a literal dollar sign, use `\$`
- **Alignment** - Use `\quad` for spacing or `\begin{aligned}...\end{aligned}` for multi-line alignment
- **Reference** - For complete LaTeX syntax, see the [KaTeX documentation](https://katex.org/docs/supported.html)

## Troubleshooting

**Formula not rendering?**
- Check for matching dollar signs
- Ensure LaTeX syntax is correct
- Look for unescaped special characters

**Common LaTeX commands:**
- `\frac{numerator}{denominator}` - fractions
- `\sqrt{x}` - square root
- `^` - superscript
- `_` - subscript
- `\left( ... \right)` - auto-sized parentheses
- `\text{...}` - regular text inside formulas
