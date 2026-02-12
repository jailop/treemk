# Diagrams with Mermaid

TreeMk supports creating diagrams using Mermaid syntax. Create flowcharts, sequence diagrams, Gantt charts, and more using simple text-based notation.

## Creating Diagrams

Create a Mermaid diagram using a code block with the `mermaid` language identifier:

````
```mermaid
graph TD
    A[Start] --> B{Is it working?}
    B -->|Yes| C[Great!]
    B -->|No| D[Debug]
    D --> B
```
````

**Result:**

```mermaid
graph TD
    A[Start] --> B{Is it working?}
    B -->|Yes| C[Great!]
    B -->|No| D[Debug]
    D --> B
```

## Flowcharts

### Basic Flowchart

````
```mermaid
graph TD
    A[Christmas] -->|Get money| B(Go shopping)
    B --> C{Let me think}
    C -->|One| D[Laptop]
    C -->|Two| E[iPhone]
    C -->|Three| F[Car]
```
````

```mermaid
graph TD
    A[Christmas] -->|Get money| B(Go shopping)
    B --> C{Let me think}
    C -->|One| D[Laptop]
    C -->|Two| E[iPhone]
    C -->|Three| F[Car]
```

### Node Shapes

````
```mermaid
graph LR
    A[Rectangle] --> B(Rounded)
    B --> C((Circle))
    C --> D{Diamond}
    D --> E>Flag]
    E --> F[/Parallelogram/]
```
````

```mermaid
graph LR
    A[Rectangle] --> B(Rounded)
    B --> C((Circle))
    C --> D{Diamond}
    D --> E>Flag]
    E --> F[/Parallelogram/]
```

### Flow Direction

- `graph TD` - Top to bottom (default)
- `graph LR` - Left to right
- `graph BT` - Bottom to top
- `graph RL` - Right to left

## Sequence Diagrams

Show interactions between participants over time:

````
```mermaid
sequenceDiagram
    participant User
    participant Browser
    participant Server
    participant Database
    
    User->>Browser: Enter URL
    Browser->>Server: HTTP Request
    Server->>Database: Query data
    Database-->>Server: Return results
    Server-->>Browser: HTTP Response
    Browser-->>User: Display page
```
````

```mermaid
sequenceDiagram
    participant User
    participant Browser
    participant Server
    participant Database
    
    User->>Browser: Enter URL
    Browser->>Server: HTTP Request
    Server->>Database: Query data
    Database-->>Server: Return results
    Server-->>Browser: HTTP Response
    Browser-->>User: Display page
```

### Activation and Notes

````
```mermaid
sequenceDiagram
    Alice->>+John: Hello John, how are you?
    Note right of John: John thinks
    John-->>-Alice: Great!
    Alice-)John: See you later!
```
````

```mermaid
sequenceDiagram
    Alice->>+John: Hello John, how are you?
    Note right of John: John thinks
    John-->>-Alice: Great!
    Alice-)John: See you later!
```

## Class Diagrams

Visualize object-oriented class structures:

````
```mermaid
classDiagram
    Animal <|-- Duck
    Animal <|-- Fish
    Animal : +int age
    Animal : +String gender
    Animal: +isMammal()
    Animal: +mate()
    
    class Duck{
        +String beakColor
        +swim()
        +quack()
    }
    
    class Fish{
        -int sizeInFeet
        -canEat()
    }
```
````

```mermaid
classDiagram
    Animal <|-- Duck
    Animal <|-- Fish
    Animal : +int age
    Animal : +String gender
    Animal: +isMammal()
    Animal: +mate()
    
    class Duck{
        +String beakColor
        +swim()
        +quack()
    }
    
    class Fish{
        -int sizeInFeet
        -canEat()
    }
```

## State Diagrams

Show state transitions:

````
```mermaid
stateDiagram-v2
    [*] --> Still
    Still --> [*]
    Still --> Moving
    Moving --> Still
    Moving --> Crash
    Crash --> [*]
```
````

```mermaid
stateDiagram-v2
    [*] --> Still
    Still --> [*]
    Still --> Moving
    Moving --> Still
    Moving --> Crash
    Crash --> [*]
```

## Entity Relationship Diagrams

Database relationships:

````
```mermaid
erDiagram
    CUSTOMER ||--o{ ORDER : places
    ORDER ||--|{ LINE-ITEM : contains
    CUSTOMER }|..|{ DELIVERY-ADDRESS : uses
    
    CUSTOMER {
        string name
        string email
        int customer_id
    }
    ORDER {
        int order_id
        date order_date
        string status
    }
    LINE-ITEM {
        int quantity
        decimal price
    }
```
````

```mermaid
erDiagram
    CUSTOMER ||--o{ ORDER : places
    ORDER ||--|{ LINE-ITEM : contains
    CUSTOMER }|..|{ DELIVERY-ADDRESS : uses
    
    CUSTOMER {
        string name
        string email
        int customer_id
    }
    ORDER {
        int order_id
        date order_date
        string status
    }
    LINE-ITEM {
        int quantity
        decimal price
    }
```

## Gantt Charts

Project timelines and schedules:

````
```mermaid
gantt
    title Project Development Schedule
    dateFormat  YYYY-MM-DD
    section Planning
    Requirements       :a1, 2024-01-01, 30d
    Design            :a2, after a1, 20d
    
    section Development
    Backend           :a3, 2024-02-01, 45d
    Frontend          :a4, after a3, 30d
    
    section Testing
    Unit Tests        :a5, after a4, 15d
    Integration       :a6, after a5, 10d
```
````

```mermaid
gantt
    title Project Development Schedule
    dateFormat  YYYY-MM-DD
    section Planning
    Requirements       :a1, 2024-01-01, 30d
    Design            :a2, after a1, 20d
    
    section Development
    Backend           :a3, 2024-02-01, 45d
    Frontend          :a4, after a3, 30d
    
    section Testing
    Unit Tests        :a5, after a4, 15d
    Integration       :a6, after a5, 10d
```

## Pie Charts

Data visualization:

````
```mermaid
pie title Technology Stack Usage
    "JavaScript" : 45
    "Python" : 25
    "Java" : 15
    "Go" : 10
    "Others" : 5
```
````

```mermaid
pie title Technology Stack Usage
    "JavaScript" : 45
    "Python" : 25
    "Java" : 15
    "Go" : 10
    "Others" : 5
```

## Git Graph

Version control visualization:

````
```mermaid
gitGraph
    commit
    commit
    branch develop
    checkout develop
    commit
    commit
    checkout main
    merge develop
    commit
    branch feature
    checkout feature
    commit
```
````

```mermaid
gitGraph
    commit
    commit
    branch develop
    checkout develop
    commit
    commit
    checkout main
    merge develop
    commit
    branch feature
    checkout feature
    commit
```

## User Journey

Track user experience:

````
```mermaid
journey
    title My working day
    section Go to work
      Make tea: 5: Me
      Go upstairs: 3: Me
      Do work: 1: Me, Cat
    section Go home
      Go downstairs: 5: Me
      Sit down: 5: Me
```
````

## Mindmap

Hierarchical information:

````
```mermaid
mindmap
  root((TreeMk))
    Features
      Editor
        Syntax highlighting
        Auto-completion
      Preview
        Live rendering
        Themes
      Export
        PDF
        HTML
        DOCX
    Benefits
      Productivity
      Organization
      Collaboration
```
````

## Timeline

Chronological events:

````
```mermaid
timeline
    title History of Software Development
    1970s : Unix created
          : C programming language
    1980s : Object-oriented programming
          : GUI interfaces
    1990s : Internet era
          : Open source movement
    2000s : Mobile computing
          : Cloud services
    2010s : AI and Machine Learning
          : Container technology
```
````

## Tips

- **Preview** - Diagrams render in the preview pane
- **Syntax** - Check [Mermaid documentation](https://mermaid.js.org/) for complete syntax
- **Themes** - Diagrams adapt to your preview theme (light/dark)
- **Complexity** - Keep diagrams simple for better readability
- **Export** - Requires `mermaid-filter` for PDF/DOCX export

## Common Issues

**Diagram not rendering?**
- Ensure code block uses `mermaid` language identifier
- Check syntax for typos
- Verify node IDs don't contain spaces
- Look for missing arrows or brackets

**Export issues?**
- Install `mermaid-filter`: `npm install -g mermaid-filter`
- Ensure it's available in your PATH
- HTML export always works without additional tools

## Advanced Features

### Styling

You can customize diagram appearance:

````
```mermaid
graph TD
    A[Start]:::startStyle --> B[Process]:::processStyle
    classDef startStyle fill:#9f6,stroke:#333,stroke-width:2px
    classDef processStyle fill:#bbf,stroke:#333,stroke-width:4px
```
````

### Subgraphs

Group related nodes:

````
```mermaid
graph TB
    subgraph "Frontend"
        A[React] --> B[Redux]
    end
    subgraph "Backend"
        C[Node.js] --> D[Express]
    end
    B --> C
```
````

### Links and Callbacks

````
```mermaid
graph LR
    A[Documentation] --> B[TreeMk Repo]
    click B "https://github.com/example/treemk"
```
````
