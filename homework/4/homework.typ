#let homework(course: "数据库系统及应用作业", number: int(0), name: "傅申",
        id: "PB20000051", body) = {
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
  set text(font: "FandolSong", lang: "zh")

  show math.equation: set text(font: "New Computer Modern Math", weight: 400)

  set enum(numbering: "(1)")

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
  set par(justify: false)

  // Code
  show raw.where(block: true): block.with(
    inset: 0pt,
    outset: 0pt,
    radius: 4pt,
  )

  body
}

#let question_counter = counter("question")
#let question_name = "题"

#let question(number: none) = {
  if number == none {
    question_counter.step()
    strong([
      #question_counter.display(question_name + " 1.")
      #v(-0.9em)
      #line(length: 100%)
      #v(-0.4em)
    ])
  } else {
    let number = question_name + " " + str(number) + "."
    strong([#number #v(-0.9em) #line(length: 100%) #v(-0.4em)])
  }
}
