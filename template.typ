#let homework(course: "课程作业", number: int(0), name: "姓名", id: "PB2XXXXXXX",
             code_with_line_number: true, body) = {
    // Set the document's basic properties.
    let author = name + " " + id
    let title = course + " " + str(number)
    set document(author: (author, ), title: title)
    set page(
        numbering: "1",
        number-align: center,
        // Running header.
        header-ascent: 14pt,
        header: locate(loc => {
            let i = counter(page).at(loc).first()
            if i == 1 { return }
            set text(size: 8pt)
            grid(
                columns: (auto, 1fr),
                align(left, title),
                align(right, author),
            )
        }),
    )

    // Font settings
    let basic-font = "Linux Libertine"
    let chinese-base-font = "FZShuSong-Z01"
    let chinese-strong-font = "FZXiaoBiaoSong-B05"
    let chinese-emph-font = "FZKai-Z03"
    
    set text(font: (basic-font, chinese-base-font), lang: "zh")
    
    show math.equation: set text(font: "Libertinus Math", weight: 400)
    show emph: set text(font: (basic-font, chinese-emph-font))
    show strong: set text(font: (basic-font, chinese-strong-font))

    // Set paragraph spacing.
    show par: set block(above: 1.2em, below: 1.2em)

    set par(leading: 0.75em)

    // Title row.
    align(center)[
        #block[
            #set text(size: 1.75em)
            #strong(title)
        ]

        #v(0.25em)

        #author

        #v(0.25em)
    ]

    // Main body.
    set par(linebreaks: "optimized", justify: true)

    // Enum style
    set enum(numbering: "(a)")
    

    // Code
    show raw.where(block: false): box.with(
        fill: luma(240),
        inset: (x: 3pt, y: 0pt),
        outset: (y: 3pt),
        radius: 2pt,
    )

    show raw.where(block: true): block.with(
        width: 100%,
        fill: luma(240),
        inset: 10pt,
        radius: 4pt,
    )

    // Code block line numbers
    show raw.where(block: true): it => {
        if not code_with_line_number { return it }
        let lines = it.text.split("\n")
        let length = lines.len()
        let i = 0
        let left_str = while i < length {
            i = i + 1
            str(i) + "\n"
        }
        grid(
            columns: (auto, 1fr),
            align(
                right,
                block(
                    inset: (
                        top: 10pt,
                        bottom: 10pt,
                        left: 0pt,
                        right: 5pt
                    ),
                    [
                      #show par: set block(above: 1.2em, below: 1.2em)
                      #left_str
                    ]
                )
            ),
            align(left, it),
        )
    }

    body
}

#let question_counter = counter("question")

#let question(number: none, content) = {
    if number == none {
        question_counter.step()
        strong([
            #question_counter.display("1.")
            #content
            #v(-0.9em)
            #line(length: 100%)
            #v(-0.6em)
        ])
    } else {
        let number = str(number) + "."
        strong([#number #content #v(-0.9em) #line(length: 100%) #v(-0.6em)])
    }
}