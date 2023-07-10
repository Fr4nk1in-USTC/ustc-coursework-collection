#let report(course: "人工智能基础", number: int(0), name: "傅申",
        id: "PB20000051", body) = {
  // Set the document's basic properties.
  let author = name + " " + id
  let title = course + "实验 " + str(number)
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
  set text(font: "FandolSong", lang: "zh")

  show math.equation: set text(font: "New Computer Modern Math", weight: 400)
  set math.equation(numbering: "(1)")

  set enum(numbering: "(1)")

  set heading(numbering: "1.")

  // Set paragraph spacing.
  show par: set block(above: 1.2em, below: 1.2em)

  set par(leading: 0.75em)

  // Title row.
  align(center)[
    #block(text(1.75em, strong(title)))

    #v(0.25em)

    #author

    #v(0.25em)
  ]

  // Main body.
  set par(justify: true)

  // Code
  show raw: text.with(font: "JetBrainsMono Nerd Font")

  show raw.where(block: true): block.with(
    width: 100%,
    inset: 10pt,
    radius: 4pt,
    stroke: 0.75pt,
  )

  body
}
