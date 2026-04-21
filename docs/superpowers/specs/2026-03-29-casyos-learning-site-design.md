# CasyOS Learning Site Design

## Goal

Build a local learning website that helps the user understand the `CasyOS` project module by module, while also providing Markdown-based comments and note-taking that persist as files inside the repository.

The site should not behave like a generic code browser. Its primary job is to explain the system clearly, guide learning in a sensible order, and turn the user's comments into long-term study documentation.

## Why This Exists

`CasyOS` is a compact learning RTOS project, but its value is spread across multiple layers:

- project-level intent in the README
- startup flow in demo `main.c`
- exception-driven context switching in `CasyOS.c`
- scheduler and task state logic in `os_task.c`
- IPC behavior across `os_sem.c`, `os_mutex.c`, `os_q.c`, and `os_flag.c`
- portability concerns in `os_cfg.h` and chip-specific demo directories

Without a guided interface, the reader has to reconstruct the architecture mentally from scattered files. The website should reduce that load.

## Product Shape

The product will be a hybrid learning site:

- tutorial-like on the homepage and learning path
- documentation-like on module pages
- personal knowledge-base-like in comments and generated notes

This matches the user's request for both structured understanding and long-term annotation.

## Primary Users

The primary user is the repository owner learning and documenting `CasyOS`.

Secondary users may include:

- classmates or teammates learning RTOS basics
- future contributors who need a guided explanation of the codebase
- the future version of the current user, returning later to continue study

## Non-Goals

The first phase should not include:

- authentication or multi-user accounts
- remote database storage
- online collaboration features
- full generic repository browsing
- automatic parsing of every symbol in the codebase
- production deployment requirements

The work should still remain suitable for committing back to the repository and publishing to GitHub after implementation.

## Recommended Tech Stack

Use:

- `Vite + React` for the frontend
- `Node + Express` for the local backend

Reasoning:

- fast to set up in an existing non-web repository
- simple local full-stack workflow
- flexible enough for structured docs, diagrams, comments, and future enhancements
- lower overhead than Next.js for a local learning-focused tool

## Information Architecture

### 1. Home Page

The homepage should answer:

- What is `CasyOS`?
- Why does this project exist?
- What are the major modules?
- In what order should I learn them?

Planned sections:

- project summary hero
- "what you will learn" overview
- architecture map
- recommended learning path
- key capabilities and limitations
- entry points into deeper module pages

### 2. Module Pages

Each core area of the RTOS gets its own page. Every page should follow a stable structure so learning feels consistent.

Recommended page template:

1. module purpose
2. core data structures
3. key execution flow
4. related source files
5. important code anchors
6. common pitfalls / implementation notes
7. Markdown comment area
8. existing notes and comment thread

### 3. Source Guide Pages

These pages should explain important cross-file journeys, such as:

- from `main()` to `OSStart()`
- what happens during a task switch
- how `OSDelay()` affects scheduling
- how a pend/post cycle wakes a task

These pages should be written as guided walkthroughs, not just file listings.

### 4. Learning Notes Area

This area should aggregate the user's notes and comments across the site, so the learning process becomes a reusable knowledge base.

Views may include:

- by module
- by file
- by tag
- recent notes

## Module Coverage For Phase 1

The site should explicitly cover these modules:

- project overview
- startup and boot sequence
- exception model: `SysTick`, `SVC`, `PendSV`
- task model and TCB
- ready list and priority bitmap scheduler
- delay and tick update behavior
- semaphore implementation
- mutex implementation
- message queue implementation
- event flag implementation
- memory management
- configuration and portability via `os_cfg.h`
- demo projects and migration path
- known risks and code issues discovered during reading

These modules match both the repository structure and the user's learning goal.

## Content Model

The frontend should not hardcode everything into components. Use structured content files so the explanation layer can grow independently from UI code.

Recommended content categories:

- `modules`: metadata for each learning module
- `sections`: structured explanatory blocks
- `sourceRefs`: file references and relevant code anchors
- `learningPaths`: recommended reading order

Example module metadata should include:

- `id`
- `title`
- `summary`
- `category`
- `files`
- `prerequisites`
- `sections`
- `tags`

## Comment and Note Model

Comments must support Markdown.

The user specifically wants comments to become learning material, so comments should be treated as first-class study records rather than disposable UI messages.

Recommended comment fields:

- `id`
- `moduleId`
- `sectionId`
- `sourceRef`
- `contentMarkdown`
- `tags`
- `createdAt`
- `updatedAt`

Optional future fields:

- `title`
- `status`
- `linkedNoteId`

## Persistence Strategy

Persist data inside the repository as files.

Recommended layout:

- `learning-site/` for application code
- `site-data/modules/` for module metadata and structured explanation data
- `site-data/comments/` for raw comment records
- `docs/learning/` for generated Markdown learning documents

### Raw Comments

Store comments in JSON for straightforward machine read/write.

This keeps API logic simple and makes future filtering easy.

### Generated Learning Documents

The system should be able to generate Markdown documents by aggregating comments. For example:

- one Markdown file per module
- one digest for recent study notes
- one curated summary document from selected comments

This allows the user's annotations to evolve into polished learning notes.

## Backend Responsibilities

The Express server should handle:

- serving structured module data
- serving existing comments
- creating new comments
- updating comments
- deleting comments if needed
- generating Markdown note files from stored comments

The backend should write files only inside the repository's dedicated site data and docs directories.

## Frontend Responsibilities

The React app should handle:

- homepage and learning navigation
- module page rendering
- source reference display
- Markdown editing and preview
- comment listing and filtering
- generated note access

The UI should prioritize readability, not dashboard complexity.

## UI Direction

The site should feel like a serious learning workspace rather than a generic admin panel.

Recommended layout:

- homepage with stronger visual hierarchy
- content pages with a three-column learning layout where appropriate:
  - left: learning navigation
  - center: explanation content
  - right: notes/comments

On smaller screens, the layout should collapse gracefully into stacked sections.

The visual style should support long-form reading:

- clear typography
- strong section hierarchy
- code/file reference styling
- restrained color system
- diagrams where helpful
- minimal visual noise

## Key User Flows

### Flow 1: Guided Learning

1. open homepage
2. pick recommended learning path
3. enter a module page
4. read explanation and inspect source references
5. leave Markdown comments and questions

### Flow 2: Build Personal Notes

1. write comments while reading
2. tag notes by topic
3. generate or review Markdown learning notes
4. continue refining notes over time

### Flow 3: Follow Runtime Behavior

1. open a source guide page
2. inspect a key runtime journey
3. jump to relevant modules
4. annotate confusing points

## Error Handling

The first version should keep error handling simple and explicit.

Examples:

- invalid module id -> return clear API 404
- malformed comment payload -> return validation error
- failed file write -> show friendly UI message and keep editor content intact
- generated note conflict -> overwrite deterministically or version clearly

## Risks and Constraints

### 1. Encoding

Some source comments in the repository appear to use a non-UTF-8 encoding. The learning site should avoid assuming all source comments are clean UTF-8 text.

### 2. Source Accuracy

The educational summaries must stay grounded in the actual code. We should avoid inventing behavior not supported by the current source.

### 3. Scope Creep

This can easily grow into a full documentation engine. Phase 1 must stay focused on `CasyOS` comprehension plus note-taking.

## Phase 1 Deliverables

Phase 1 should include:

- a runnable local full-stack site
- homepage with learning map
- repository-friendly file layout that can be committed and pushed to GitHub
- module pages for the core `CasyOS` areas
- structured content source for those modules
- Markdown comment editor and preview
- local file persistence for comments
- generated Markdown note output

## Testing Strategy

Testing should cover:

- backend route validation
- file persistence behavior
- Markdown comment round-trip
- rendering of module pages from structured data
- at least one generated learning note workflow

If time allows, include lightweight integration tests for comment creation and note generation.

## File and Directory Plan

Recommended additions:

- `D:/explore/CasyOS/learning-site/`
- `D:/explore/CasyOS/site-data/modules/`
- `D:/explore/CasyOS/site-data/comments/`
- `D:/explore/CasyOS/docs/learning/`

## Recommended Implementation Order

1. scaffold the web app and backend
2. define content schema and module metadata
3. build homepage and module page shell
4. add Markdown comment editor and preview
5. implement file persistence API
6. generate Markdown learning docs
7. polish navigation and reading experience

## Acceptance Criteria

The design will be considered successful when:

- the user can learn `CasyOS` in a structured order from the site
- each important kernel module has a dedicated explanation page
- comments can be written in Markdown
- comments persist as repository files
- comments can be turned into learning documents
- the site feels like a focused study tool, not a generic code viewer
- the final project state is ready to be stored in the GitHub repository

## Open Questions Resolved

- Site style: hybrid tutorial + documentation
- Comment persistence: repository files
- Comment format: Markdown
- Architecture: local frontend + local backend
- Scope: learning-first, not production-first
- Final storage: commitable and pushable to GitHub after implementation

## Recommendation

Proceed with a Phase 1 implementation using `Vite + React + Node + Express`, with structured module content and Markdown comments stored as files in the repository.
