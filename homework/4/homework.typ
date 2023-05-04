#let homework(course: "人工智能基础作业", number: int(0), name: "傅申", id: "PB20000051",
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
        columns: (50%, 50%),
        align(left, title),
        align(right, author),
      )
    }),
  )
  set text(font: ("FandolSong"), lang: "zh")
  show math.equation: set text(font: ("New Computer Modern Math", "FandolSong"), weight: 400)

  // Set paragraph spacing.
  show par: set block(above: 1.2em, below: 1.2em)

  set par(leading: 0.75em)

  // Title row.
  align(center)[
    #block(text(weight: 700, 1.75em, title))

    #v(0.25em)

    #author

    #v(0.25em)
  ]

  // Main body.
  set par(justify: true)

  // Code
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

#let question(number) = {
  let number = str(number) + "."
  strong([#number #v(-0.9em) #line(length: 100%) #v(-0.6em)])
}
