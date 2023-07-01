#import("template.typ"): *

#show: report.with()

#outline(
    title: [目录],
    indent: true,
)

#pagebreak()

= 概述

== 系统目标

本实验目标是开发一个面向教师的教学科研登记系统。

== 需求说明

=== 数据需求 <data-requirements>

教师教学科研登记系统的数据需求如下：

#figure(
  image("images/data-requirements.png"),
  caption: [教师教学科研登记系统的数据需求]
)

其中：
1. 性别为整数，1 - 男，2 - 女；
2. 教师职称为整数：1 - 博士后，2 - 助教，3 - 讲师，4 - 副教授，5 - 特任教授，6 - 教授，7 - 助理研究员，8 - 特任副研究员，9 - 副研究员，10 - 特任研究员，11 - 研究员；
3. 论文类型为整数：1 - full paper，2 - short paper，3 - poster paper，4 - demo paper；
4. 论文级别为整数：1 - CCF-A，2 - CCF-B，3 - CCF-C，4 - 中文 CCF-A，5 - 中文 CCF-B，6 - 无级别；
5. 项目类型为整数：1 - 国家级项目，2 - 省部级项目，3 - 市厅级项目，4 - 企业合作项目，5 - 其它类型项目；
6. 发表论文和承担项目中的排名：1 - 表示排名第一，以此类推。论文排名第一即为第一作者，承担项目排名第一即为项目负责人；
7. 主讲课程中的学期取值为：1 - 春季学期，2 - 夏季学期，3 - 秋季学期；
8. 课程性质为整数：1 - 本科生课程，2 - 研究生课程。

=== 功能需求

要求实现的主要功能有：
/ 登记发表论文情况: 提供教师论文发表信息的的增、删、改、查功能；输入时要求检查：一篇论文只能有一位通讯作者，论文的作者排名不能有重复，论文的类型和级别只能在约定的取值集合中选取（实现时建议用下拉框）。
/ 登记承担项目情况: 提供教师承担项目信息的增、删、改、查功能；输入时要求检查：排名不能有重复，一个项目中所有教师的承担经费总额应等于项目的总经费，项目类型只能在约定的取值集合中选取。 
/ 登记主讲课程情况: 提供教师主讲课程信息的增、删、改、查功能；输入时要求检查：一门课程所有教师的主讲学时总额应等于课程的总学时，学期。
/ 查询统计: 实现按教师工号和给定年份范围汇总查询该教师的教学科研情况的功能；例如输入工号“01234”，“2023-2023”可以查询 01234 教师在 2023 年度的教学科研工作情况。

== 本报告的主要贡献

本报告是对此次实验结果（即最后得到的应用程序）的设计、实现和功能的补充说明，并给出了相应的示例。

= 总体设计

== 系统模块结构

如下页@framework 所示。

== 系统工作流程

如@procedure 所示：

#figure(
  image("images/procedure.svg", width: 95%),
  caption: [系统工作流程]
)<procedure>

其中，只有在前期准备中成功连接到数据库之后，用户才能进行 CURD 操作，否则页面上只会显示错误信息，只能进行重连操作。

#figure(
  image("images/framework.svg", width: 95%),
  caption: [系统模块结构]
)<framework>

#pagebreak()

== 数据库设计

数据库结构如下@database 所示，其中每个方框最上面一栏为表名，中间一栏为非主码字段，最下面一栏为主码字段：

#figure(
  image("images/database.svg", width: 95%),
  caption: [数据库结构]
)<database>


与@data-requirements 中不同的是，论文表 `Paper` 的发表年份字段 `year` 的类型为
`int` 而非 `Date`，这是为了方便在前后端传递数据；同时，论文表 `Paper` 的论文号字段 `id`
被设置为了自增主码，用户不需要输入，也不被允许对该字段进行修改。

= 实现与测试

== 实现结果

程序将连接的数据库 URL 存储在一个 YAML 配置文件中, 在我的 Linux 笔记本上，该文件位于
`$HOME/.config/teacher-record-app/config.yml`。在第一次打开应用程序时，会提示没有配置
URL，这时只需要修改对应的配置文件即可（配置文件路径会显示在应用程序中）。配置文件的格式为

```yaml
database_url: "mysql://username:password@host/database"
```

=== 论文管理

论文管理页面的截图如@paper-page，其中上半部分为查询模块，下半部分为数据显示模块（同时分担增、改和删功能）：
- 在查询模块中，可以只输入部分字段进行查询，并且对于类型为字符串的字段，查询将采用前缀匹配的原则，不需要完全匹配。查询功能的结果如@paper-search。
- 点击“新增论文”按钮，将弹出一个对话框，如@paper-add 所示，用户将输入论文的所有字段，并在“论文教师”部分编辑论文的教师。
- 点击表格右侧的“查看”按钮，将弹出一个对话框，其中有论文的详细信息，如@paper-detail 所示。
- 点击表格右侧的“修改”按钮，将弹出一个对话框，用于修改论文的信息，如@paper-edit 所示。
- 点击表格右侧的“删除”按钮，将弹出一个消息弹窗，用于确认是否删除该论文，如@paper-delete。
- 在选中部分论文后，点击“删除选中”按钮，将弹出一个消息弹窗，用于确认是否删除这些论文，与上一点中类似。

#figure(
  image("images/paper/page.png", height: 27%),
  caption: [论文管理页面]
)<paper-page>

#figure(
  image("images/paper/search.png", height: 27%),
  caption: [论文查询]
)<paper-search>

#figure(
  image("images/paper/add.png", height: 27%),
  caption: [新增论文]
)<paper-add>

#figure(
  image("images/paper/detail.png", height: 29%),
  caption: [论文详细信息]
)<paper-detail>

#figure(
  image("images/paper/edit.png", height: 29%),
  caption: [修改论文]
)<paper-edit>

#figure(
  image("images/paper/delete.png", height: 29%),
  caption: [删除论文]
)<paper-delete>

=== 项目管理

项目管理页面的截图如@project-page，与论文管理页面类似，操作和功能也类似，这里只讨论不同的地方：在项目添加/修改对话框中，不允许直接编辑项目经费，项目经费将由所有教师承担经费求和得到，如@project-add 和@project-edit 所示。

#figure(
  image("images/project/page.png", height: 25%),
  caption: [项目管理页面]
)<project-page>

#figure(
  image("images/project/add.png", height: 25%),
  caption: [新增项目]
)<project-add>

#figure(
  image("images/project/edit.png", height: 25%),
  caption: [修改项目]
)<project-edit>

=== 教学管理

教学管理的页面截图如@teaching-page，与上面的页面也类似。

与上面的页面不同的是，在首次进入该页面时，会显示所有的课程信息，但这些信息是不可编辑的（因为无法直接编辑课程信息）。只有在选定了年份和学期后（需要按“查询”按钮才会刷新表格），才允许修改教学信息（`Teacher_Course` 表格），如<teaching-selected>。此时，修改的教学信息是对应年份和学期的教学信息。

修改操作与上面类似，这里只说明不同的地方：在新增/修改教学信息时，需要保证教师承担的总学时不能超过课程学时，否则无法修改（会有错误信息提示）。

#figure(
  image("images/teaching/page.png", height: 30%),
  caption: [教学管理页面]
)<teaching-page>

#figure(
  image("images/teaching/selected.png", height: 30%),
  caption: [选定年份和学期后的教学管理页面]
)<teaching-selected>

=== 教师管理

教师管理的页面截图如@teacher-page，与上面的页面也类似，不在赘述，这里主要讨论查询统计功能：在输入合法的工号和年份后点击“查询”按钮，将弹出对话框显示对应的统计信息，如<summary>。

#figure(
  image("images/teacher/page.png"),
  caption: [教师管理页面]
)<teacher-page>

#figure(
  image("images/teacher/summary.png"),
  caption: [教师管理页面]
)<summary>

== 测试结果

具体的测试方式在上面或多或少都体现了一部分，下面将主要展示各种非法输入的处理。

=== 不存在的教师工号

以新增论文为例，当输入一个不存在的教师工号时，将在对应的输入栏提示该工号不存在，如@id-not-exist 所示。

#figure(
  image("images/error/id-not-exist.png"),
  caption: [不存在的教师工号]
)<id-not-exist>

=== 未输入必须项

以新增项目为例，当未输入必须项时，将在对应的输入栏提示该项为必须项，如@required 所示。

#figure(
  image("images/error/required.png"),
  caption: [未输入必须项]
)<required>

=== 重复的 ID（教师工号、项目号）

以新增项目为例，当输入的项目号重复时，将在对应的输入栏提示该项目号已存在，如@duplicated-id 所示。

#figure(
  image("images/error/duplicated-id.png"),
  caption: [项目号重复]
)<duplicated-id>

=== 起始年份大于结束年份

以新增项目为例，当输入的起始年份大于结束年份时，将在对应的输入栏提示该信息，如@invalid-year 所示。

#figure(
  image("images/error/invalid-year.png"),
  caption: [起始年份大于结束年份]
)<invalid-year>

=== 总承担学时不等于课程学时

在新增/修改教学信息时，如果总承担学时不等于课程学时，将弹出错误消息，如@invalid-credit 所示。

#figure(
  image("images/error/invalid-credit.png"),
  caption: [总承担学时不等于课程学时]
)<invalid-credit>

= 总结与讨论

本次实验实现了一个与数据库交互的应用程序的前后端，实现了对教师、论文、项目、教学信息的管理，以及对教师的查询统计功能。

在实现过程中，我遇到了很多问题，比如如何将数据库中的数据与后端的数据绑定，在前后端之间传递信息，通过查阅文档和参考官方示例，最终解决了这些问题。

= 附录

若要编译程序，首先确保拥有 Rust 和 NPM 环境，然后在 `teacher-record-app` 目录执行

```sh
npm install
cargo install tauri-cli
cargo tauri build
```

即可在 `src-tauri/target/release/` 目录下找到可执行程序。
