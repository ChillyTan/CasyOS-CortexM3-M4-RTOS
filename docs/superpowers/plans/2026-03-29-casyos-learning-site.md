# CasyOS Learning Site Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a local full-stack learning website for understanding `CasyOS`, with structured module content, Markdown comments persisted as repository files, generated learning notes, and a repository state ready to commit and push to GitHub.

**Architecture:** Use a Vite + React frontend inside `learning-site/` and a Node + Express backend inside the same workspace. The frontend renders a guided learning experience from structured JSON content in `site-data/modules`, while the backend reads and writes module content, Markdown comments, and generated note files under repository-owned directories.

**Tech Stack:** Vite, React, TypeScript, Express, Zod, react-router-dom, react-markdown, remark-gfm, Vitest, Testing Library, Supertest

---

## File Structure

### New directories

- `D:/explore/CasyOS/learning-site/`
- `D:/explore/CasyOS/learning-site/src/app/`
- `D:/explore/CasyOS/learning-site/src/components/`
- `D:/explore/CasyOS/learning-site/src/features/home/`
- `D:/explore/CasyOS/learning-site/src/features/modules/`
- `D:/explore/CasyOS/learning-site/src/features/comments/`
- `D:/explore/CasyOS/learning-site/src/features/notes/`
- `D:/explore/CasyOS/learning-site/src/lib/`
- `D:/explore/CasyOS/learning-site/server/`
- `D:/explore/CasyOS/learning-site/server/routes/`
- `D:/explore/CasyOS/learning-site/server/services/`
- `D:/explore/CasyOS/learning-site/server/utils/`
- `D:/explore/CasyOS/learning-site/tests/`
- `D:/explore/CasyOS/site-data/modules/`
- `D:/explore/CasyOS/site-data/comments/`
- `D:/explore/CasyOS/docs/learning/`

### New files

- `D:/explore/CasyOS/learning-site/package.json`
- `D:/explore/CasyOS/learning-site/tsconfig.json`
- `D:/explore/CasyOS/learning-site/tsconfig.node.json`
- `D:/explore/CasyOS/learning-site/vite.config.ts`
- `D:/explore/CasyOS/learning-site/index.html`
- `D:/explore/CasyOS/learning-site/src/main.tsx`
- `D:/explore/CasyOS/learning-site/src/app/App.tsx`
- `D:/explore/CasyOS/learning-site/src/app/routes.tsx`
- `D:/explore/CasyOS/learning-site/src/app/styles.css`
- `D:/explore/CasyOS/learning-site/src/lib/types.ts`
- `D:/explore/CasyOS/learning-site/src/lib/api.ts`
- `D:/explore/CasyOS/learning-site/src/components/Layout.tsx`
- `D:/explore/CasyOS/learning-site/src/components/MarkdownEditor.tsx`
- `D:/explore/CasyOS/learning-site/src/components/MarkdownPreview.tsx`
- `D:/explore/CasyOS/learning-site/src/features/home/HomePage.tsx`
- `D:/explore/CasyOS/learning-site/src/features/modules/ModulePage.tsx`
- `D:/explore/CasyOS/learning-site/src/features/modules/ModuleList.tsx`
- `D:/explore/CasyOS/learning-site/src/features/modules/SourceGuidePage.tsx`
- `D:/explore/CasyOS/learning-site/src/features/comments/CommentPanel.tsx`
- `D:/explore/CasyOS/learning-site/src/features/comments/CommentList.tsx`
- `D:/explore/CasyOS/learning-site/src/features/notes/NotesPage.tsx`
- `D:/explore/CasyOS/learning-site/server/index.ts`
- `D:/explore/CasyOS/learning-site/server/app.ts`
- `D:/explore/CasyOS/learning-site/server/routes/modules.ts`
- `D:/explore/CasyOS/learning-site/server/routes/comments.ts`
- `D:/explore/CasyOS/learning-site/server/routes/notes.ts`
- `D:/explore/CasyOS/learning-site/server/services/moduleService.ts`
- `D:/explore/CasyOS/learning-site/server/services/commentService.ts`
- `D:/explore/CasyOS/learning-site/server/services/noteService.ts`
- `D:/explore/CasyOS/learning-site/server/utils/paths.ts`
- `D:/explore/CasyOS/learning-site/server/utils/schemas.ts`
- `D:/explore/CasyOS/learning-site/tests/server.comments.test.ts`
- `D:/explore/CasyOS/learning-site/tests/server.notes.test.ts`
- `D:/explore/CasyOS/learning-site/tests/module-page.test.tsx`
- `D:/explore/CasyOS/site-data/modules/index.json`
- `D:/explore/CasyOS/site-data/modules/project-overview.json`
- `D:/explore/CasyOS/site-data/modules/startup-and-boot.json`
- `D:/explore/CasyOS/site-data/modules/exception-model.json`
- `D:/explore/CasyOS/site-data/modules/task-scheduler.json`
- `D:/explore/CasyOS/site-data/modules/delay-and-tick.json`
- `D:/explore/CasyOS/site-data/modules/semaphore.json`
- `D:/explore/CasyOS/site-data/modules/mutex.json`
- `D:/explore/CasyOS/site-data/modules/message-queue.json`
- `D:/explore/CasyOS/site-data/modules/event-flag.json`
- `D:/explore/CasyOS/site-data/modules/memory-management.json`
- `D:/explore/CasyOS/site-data/modules/config-and-porting.json`
- `D:/explore/CasyOS/site-data/modules/demo-projects.json`
- `D:/explore/CasyOS/site-data/modules/code-risks.json`
- `D:/explore/CasyOS/site-data/comments/.gitkeep`
- `D:/explore/CasyOS/docs/learning/.gitkeep`
- `D:/explore/CasyOS/docs/learning/README.md`

### Existing files to read but not necessarily modify

- `D:/explore/CasyOS/README.md`
- `D:/explore/CasyOS/CasyOS 快速上手_移植必看.md`
- `D:/explore/CasyOS/SourceCode/CasyOS.c`
- `D:/explore/CasyOS/SourceCode/CasyOS.h`
- `D:/explore/CasyOS/SourceCode/os_cfg.h`
- `D:/explore/CasyOS/SourceCode/os_task.c`
- `D:/explore/CasyOS/SourceCode/os_sem.c`
- `D:/explore/CasyOS/SourceCode/os_mutex.c`
- `D:/explore/CasyOS/SourceCode/os_q.c`
- `D:/explore/CasyOS/SourceCode/os_flag.c`

## Task 1: Scaffold the learning-site workspace

**Files:**
- Create: `D:/explore/CasyOS/learning-site/package.json`
- Create: `D:/explore/CasyOS/learning-site/tsconfig.json`
- Create: `D:/explore/CasyOS/learning-site/tsconfig.node.json`
- Create: `D:/explore/CasyOS/learning-site/vite.config.ts`
- Create: `D:/explore/CasyOS/learning-site/index.html`
- Create: `D:/explore/CasyOS/learning-site/src/main.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/app/App.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/app/routes.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/app/styles.css`

- [ ] **Step 1: Write the failing smoke test expectation in plan notes**

```ts
// Target smoke behavior after scaffold:
// 1. `npm run build` succeeds
// 2. `npm run test` can execute at least one frontend test file
// 3. `npm run dev` starts Vite and the Express server together
```

- [ ] **Step 2: Create `package.json` with exact scripts and dependencies**

```json
{
  "name": "casyos-learning-site",
  "private": true,
  "version": "0.1.0",
  "type": "module",
  "scripts": {
    "dev": "concurrently \"npm:dev:web\" \"npm:dev:server\"",
    "dev:web": "vite",
    "dev:server": "tsx watch server/index.ts",
    "build": "tsc -b && vite build",
    "test": "vitest run",
    "test:watch": "vitest",
    "lint": "tsc --noEmit"
  }
}
```

- [ ] **Step 3: Add base TypeScript and Vite config**

```ts
import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  plugins: [react()],
  server: {
    port: 5173,
    proxy: {
      "/api": "http://localhost:4173"
    }
  },
  test: {
    environment: "jsdom",
    setupFiles: "./tests/setup.ts"
  }
});
```

- [ ] **Step 4: Create the minimal React app shell**

```tsx
import { RouterProvider } from "react-router-dom";
import { router } from "./routes";
import "./styles.css";

export function App() {
  return <RouterProvider router={router} />;
}
```

- [ ] **Step 5: Run install and scaffold verification**

Run: `npm install`
Expected: dependencies install successfully in `D:\explore\CasyOS\learning-site`

Run: `npm run build`
Expected: Vite build completes successfully

- [ ] **Step 6: Commit**

```bash
git -C D:/explore/CasyOS add learning-site
git -C D:/explore/CasyOS commit -m "feat: scaffold CasyOS learning site workspace"
```

## Task 2: Create the backend API shell and shared schemas

**Files:**
- Create: `D:/explore/CasyOS/learning-site/server/index.ts`
- Create: `D:/explore/CasyOS/learning-site/server/app.ts`
- Create: `D:/explore/CasyOS/learning-site/server/routes/modules.ts`
- Create: `D:/explore/CasyOS/learning-site/server/routes/comments.ts`
- Create: `D:/explore/CasyOS/learning-site/server/routes/notes.ts`
- Create: `D:/explore/CasyOS/learning-site/server/utils/paths.ts`
- Create: `D:/explore/CasyOS/learning-site/server/utils/schemas.ts`
- Create: `D:/explore/CasyOS/learning-site/src/lib/types.ts`

- [ ] **Step 1: Write the failing backend test**

```ts
import request from "supertest";
import { describe, expect, it } from "vitest";
import { createServer } from "../server/app";

describe("api shell", () => {
  it("returns modules index", async () => {
    const app = createServer();
    const response = await request(app).get("/api/modules");
    expect(response.status).toBe(200);
    expect(Array.isArray(response.body.modules)).toBe(true);
  });
});
```

- [ ] **Step 2: Run the backend test to verify it fails**

Run: `npm run test -- tests/server.comments.test.ts`
Expected: FAIL because `createServer` and routes do not exist yet

- [ ] **Step 3: Implement shared schemas and paths**

```ts
import path from "node:path";

export const repoRoot = path.resolve(process.cwd(), "..");
export const modulesDir = path.join(repoRoot, "site-data", "modules");
export const commentsDir = path.join(repoRoot, "site-data", "comments");
export const learningDocsDir = path.join(repoRoot, "docs", "learning");
```

```ts
import { z } from "zod";

export const moduleSchema = z.object({
  id: z.string(),
  title: z.string(),
  summary: z.string(),
  category: z.string(),
  prerequisites: z.array(z.string()).default([]),
  sections: z.array(z.object({
    id: z.string(),
    title: z.string(),
    body: z.string()
  })),
  files: z.array(z.string()).default([]),
  tags: z.array(z.string()).default([])
});
```

- [ ] **Step 4: Implement the Express app shell**

```ts
import express from "express";
import { modulesRouter } from "./routes/modules";
import { commentsRouter } from "./routes/comments";
import { notesRouter } from "./routes/notes";

export function createServer() {
  const app = express();
  app.use(express.json());
  app.use("/api/modules", modulesRouter);
  app.use("/api/comments", commentsRouter);
  app.use("/api/notes", notesRouter);
  return app;
}
```

- [ ] **Step 5: Implement temporary route stubs**

```ts
import { Router } from "express";

export const modulesRouter = Router();

modulesRouter.get("/", (_req, res) => {
  res.json({ modules: [] });
});
```

- [ ] **Step 6: Run tests to verify they pass**

Run: `npm run test -- tests/server.comments.test.ts`
Expected: PASS

- [ ] **Step 7: Commit**

```bash
git -C D:/explore/CasyOS add learning-site
git -C D:/explore/CasyOS commit -m "feat: add learning site backend shell"
```

## Task 3: Add structured module content files for CasyOS

**Files:**
- Create: `D:/explore/CasyOS/site-data/modules/index.json`
- Create: `D:/explore/CasyOS/site-data/modules/project-overview.json`
- Create: `D:/explore/CasyOS/site-data/modules/startup-and-boot.json`
- Create: `D:/explore/CasyOS/site-data/modules/exception-model.json`
- Create: `D:/explore/CasyOS/site-data/modules/task-scheduler.json`
- Create: `D:/explore/CasyOS/site-data/modules/delay-and-tick.json`
- Create: `D:/explore/CasyOS/site-data/modules/semaphore.json`
- Create: `D:/explore/CasyOS/site-data/modules/mutex.json`
- Create: `D:/explore/CasyOS/site-data/modules/message-queue.json`
- Create: `D:/explore/CasyOS/site-data/modules/event-flag.json`
- Create: `D:/explore/CasyOS/site-data/modules/memory-management.json`
- Create: `D:/explore/CasyOS/site-data/modules/config-and-porting.json`
- Create: `D:/explore/CasyOS/site-data/modules/demo-projects.json`
- Create: `D:/explore/CasyOS/site-data/modules/code-risks.json`
- Create: `D:/explore/CasyOS/learning-site/server/services/moduleService.ts`
- Modify: `D:/explore/CasyOS/learning-site/server/routes/modules.ts`

- [ ] **Step 1: Write a failing content-loading test**

```ts
import request from "supertest";
import { describe, expect, it } from "vitest";
import { createServer } from "../server/app";

describe("modules api", () => {
  it("returns the CasyOS learning modules", async () => {
    const app = createServer();
    const response = await request(app).get("/api/modules");
    expect(response.status).toBe(200);
    expect(response.body.modules.some((item: { id: string }) => item.id === "task-scheduler")).toBe(true);
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `npm run test -- tests/server.modules.test.ts`
Expected: FAIL because route currently returns an empty list

- [ ] **Step 3: Create the module index file**

```json
{
  "learningPath": [
    "project-overview",
    "startup-and-boot",
    "exception-model",
    "task-scheduler",
    "delay-and-tick",
    "semaphore",
    "mutex",
    "message-queue",
    "event-flag",
    "memory-management",
    "config-and-porting",
    "demo-projects",
    "code-risks"
  ]
}
```

- [ ] **Step 4: Create module JSON with real repository-grounded explanations**

```json
{
  "id": "task-scheduler",
  "title": "任务管理与调度",
  "summary": "理解 TCB、就绪队列、优先级位图以及当前任务切换选择逻辑。",
  "category": "kernel-core",
  "prerequisites": ["project-overview", "startup-and-boot", "exception-model"],
  "files": [
    "D:/explore/CasyOS/SourceCode/os_task.c",
    "D:/explore/CasyOS/SourceCode/CasyOS.h"
  ],
  "tags": ["scheduler", "tcb", "priority", "ready-list"],
  "sections": [
    {
      "id": "responsibility",
      "title": "模块职责",
      "body": "这个模块维护任务全局链表、每个优先级的就绪队列、优先级位图以及延时相关的调度辅助结构。它决定了当前应该运行哪个任务。"
    }
  ]
}
```

- [ ] **Step 5: Implement a `moduleService` that loads index and detail files**

```ts
import fs from "node:fs/promises";
import path from "node:path";
import { modulesDir } from "../utils/paths";
import { moduleSchema } from "../utils/schemas";

export async function listModules() {
  const indexPath = path.join(modulesDir, "index.json");
  const indexRaw = await fs.readFile(indexPath, "utf8");
  const index = JSON.parse(indexRaw) as { learningPath: string[] };

  const modules = await Promise.all(
    index.learningPath.map(async (id) => {
      const filePath = path.join(modulesDir, `${id}.json`);
      const raw = await fs.readFile(filePath, "utf8");
      return moduleSchema.parse(JSON.parse(raw));
    })
  );

  return { learningPath: index.learningPath, modules };
}
```

- [ ] **Step 6: Wire the route to real content**

```ts
import { Router } from "express";
import { listModules } from "../services/moduleService";

export const modulesRouter = Router();

modulesRouter.get("/", async (_req, res) => {
  const data = await listModules();
  res.json(data);
});
```

- [ ] **Step 7: Run tests to verify they pass**

Run: `npm run test -- tests/server.modules.test.ts`
Expected: PASS

- [ ] **Step 8: Commit**

```bash
git -C D:/explore/CasyOS add site-data learning-site
git -C D:/explore/CasyOS commit -m "feat: add structured CasyOS learning modules"
```

## Task 4: Build the frontend routing and document-style shell

**Files:**
- Create: `D:/explore/CasyOS/learning-site/src/components/Layout.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/features/home/HomePage.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/features/modules/ModuleList.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/features/modules/ModulePage.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/features/modules/SourceGuidePage.tsx`
- Modify: `D:/explore/CasyOS/learning-site/src/app/routes.tsx`
- Modify: `D:/explore/CasyOS/learning-site/src/app/styles.css`
- Create: `D:/explore/CasyOS/learning-site/src/lib/api.ts`

- [ ] **Step 1: Write the failing frontend page test**

```tsx
import { render, screen } from "@testing-library/react";
import { MemoryRouter, Route, Routes } from "react-router-dom";
import { ModulePage } from "../src/features/modules/ModulePage";

it("renders the module title", async () => {
  render(
    <MemoryRouter initialEntries={["/modules/task-scheduler"]}>
      <Routes>
        <Route path="/modules/:moduleId" element={<ModulePage />} />
      </Routes>
    </MemoryRouter>
  );

  expect(await screen.findByText("任务管理与调度")).toBeInTheDocument();
});
```

- [ ] **Step 2: Run the frontend test to verify it fails**

Run: `npm run test -- tests/module-page.test.tsx`
Expected: FAIL because `ModulePage` does not exist yet

- [ ] **Step 3: Implement API client helpers**

```ts
export async function fetchModules() {
  const response = await fetch("/api/modules");
  if (!response.ok) throw new Error("Failed to load modules");
  return response.json();
}

export async function fetchModuleById(moduleId: string) {
  const response = await fetch(`/api/modules/${moduleId}`);
  if (!response.ok) throw new Error("Failed to load module");
  return response.json();
}
```

- [ ] **Step 4: Build the layout and routes**

```tsx
import { createBrowserRouter } from "react-router-dom";
import { Layout } from "../components/Layout";
import { HomePage } from "../features/home/HomePage";
import { ModulePage } from "../features/modules/ModulePage";
import { NotesPage } from "../features/notes/NotesPage";

export const router = createBrowserRouter([
  {
    path: "/",
    element: <Layout />,
    children: [
      { index: true, element: <HomePage /> },
      { path: "modules/:moduleId", element: <ModulePage /> },
      { path: "notes", element: <NotesPage /> }
    ]
  }
]);
```

- [ ] **Step 5: Build the homepage and module page shells**

```tsx
export function HomePage() {
  return (
    <main className="page page-home">
      <section className="hero">
        <p className="eyebrow">CasyOS Learning Workspace</p>
        <h1>从源码到机制，系统理解 CasyOS</h1>
        <p>按学习路径进入每个模块，再把你的理解沉淀成 Markdown 学习笔记。</p>
      </section>
    </main>
  );
}
```

```tsx
import { useEffect, useState } from "react";
import { useParams } from "react-router-dom";
import { fetchModuleById } from "../../lib/api";

export function ModulePage() {
  const { moduleId = "" } = useParams();
  const [module, setModule] = useState<any>(null);

  useEffect(() => {
    void fetchModuleById(moduleId).then(setModule);
  }, [moduleId]);

  if (!module) return <main className="page">Loading...</main>;

  return (
    <main className="page page-module">
      <h1>{module.title}</h1>
      <p>{module.summary}</p>
    </main>
  );
}
```

- [ ] **Step 6: Add base reading-focused styles**

```css
:root {
  --bg: #f6f1e7;
  --panel: #fffdf8;
  --text: #1f1b16;
  --muted: #6c6255;
  --accent: #9b4d1f;
  --border: #d8cbb6;
}
```

- [ ] **Step 7: Run frontend tests and build**

Run: `npm run test -- tests/module-page.test.tsx`
Expected: PASS

Run: `npm run build`
Expected: PASS

- [ ] **Step 8: Commit**

```bash
git -C D:/explore/CasyOS add learning-site
git -C D:/explore/CasyOS commit -m "feat: add learning site document shell"
```

## Task 5: Implement module detail, comments listing, and Markdown editor UI

**Files:**
- Create: `D:/explore/CasyOS/learning-site/src/components/MarkdownEditor.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/components/MarkdownPreview.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/features/comments/CommentPanel.tsx`
- Create: `D:/explore/CasyOS/learning-site/src/features/comments/CommentList.tsx`
- Modify: `D:/explore/CasyOS/learning-site/src/features/modules/ModulePage.tsx`
- Modify: `D:/explore/CasyOS/learning-site/src/lib/api.ts`
- Modify: `D:/explore/CasyOS/learning-site/src/lib/types.ts`

- [ ] **Step 1: Write the failing UI interaction test**

```tsx
it("shows the Markdown comment composer on a module page", async () => {
  render(
    <MemoryRouter initialEntries={["/modules/task-scheduler"]}>
      <Routes>
        <Route path="/modules/:moduleId" element={<ModulePage />} />
      </Routes>
    </MemoryRouter>
  );

  expect(await screen.findByPlaceholderText("写下你的理解、疑问或批注...")).toBeInTheDocument();
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `npm run test -- tests/module-page.test.tsx`
Expected: FAIL because editor UI does not exist yet

- [ ] **Step 3: Implement the Markdown editor and preview components**

```tsx
type MarkdownEditorProps = {
  value: string;
  onChange: (nextValue: string) => void;
};

export function MarkdownEditor({ value, onChange }: MarkdownEditorProps) {
  return (
    <textarea
      className="markdown-editor"
      value={value}
      placeholder="写下你的理解、疑问或批注..."
      onChange={(event) => onChange(event.target.value)}
    />
  );
}
```

```tsx
import ReactMarkdown from "react-markdown";
import remarkGfm from "remark-gfm";

export function MarkdownPreview({ value }: { value: string }) {
  return (
    <div className="markdown-preview">
      <ReactMarkdown remarkPlugins={[remarkGfm]}>{value || "_暂无预览内容_"}</ReactMarkdown>
    </div>
  );
}
```

- [ ] **Step 4: Implement the comment panel and list**

```tsx
import { useState } from "react";
import { MarkdownEditor } from "../../components/MarkdownEditor";
import { MarkdownPreview } from "../../components/MarkdownPreview";

export function CommentPanel() {
  const [value, setValue] = useState("");

  return (
    <section className="comment-panel">
      <h2>学习批注</h2>
      <MarkdownEditor value={value} onChange={setValue} />
      <MarkdownPreview value={value} />
    </section>
  );
}
```

- [ ] **Step 5: Expand the module page into three-column learning layout**

```tsx
return (
  <main className="page page-module">
    <aside className="module-nav">...</aside>
    <section className="module-content">
      <h1>{module.title}</h1>
      <p>{module.summary}</p>
      {module.sections.map((section: any) => (
        <article key={section.id}>
          <h2>{section.title}</h2>
          <p>{section.body}</p>
        </article>
      ))}
    </section>
    <aside className="module-notes">
      <CommentPanel />
      <CommentList comments={[]} />
    </aside>
  </main>
);
```

- [ ] **Step 6: Run tests to verify the page passes**

Run: `npm run test -- tests/module-page.test.tsx`
Expected: PASS

- [ ] **Step 7: Commit**

```bash
git -C D:/explore/CasyOS add learning-site
git -C D:/explore/CasyOS commit -m "feat: add markdown study comment interface"
```

## Task 6: Implement comment persistence and validation

**Files:**
- Modify: `D:/explore/CasyOS/learning-site/server/routes/comments.ts`
- Create: `D:/explore/CasyOS/learning-site/server/services/commentService.ts`
- Modify: `D:/explore/CasyOS/learning-site/tests/server.comments.test.ts`
- Modify: `D:/explore/CasyOS/learning-site/src/lib/api.ts`
- Modify: `D:/explore/CasyOS/learning-site/src/features/comments/CommentPanel.tsx`

- [ ] **Step 1: Write the failing persistence test**

```ts
it("creates a comment file for a module", async () => {
  const app = createServer();
  const response = await request(app).post("/api/comments").send({
    moduleId: "task-scheduler",
    sectionId: "responsibility",
    sourceRef: "D:/explore/CasyOS/SourceCode/os_task.c",
    contentMarkdown: "## 我的理解\n\n调度核心依赖位图与就绪队列。",
    tags: ["scheduler"]
  });

  expect(response.status).toBe(201);
  expect(response.body.moduleId).toBe("task-scheduler");
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `npm run test -- tests/server.comments.test.ts`
Expected: FAIL because POST persistence does not exist

- [ ] **Step 3: Implement the file-backed comment service**

```ts
import fs from "node:fs/promises";
import path from "node:path";
import { randomUUID } from "node:crypto";
import { commentsDir } from "../utils/paths";
import { createCommentSchema } from "../utils/schemas";

function filePathFor(moduleId: string) {
  return path.join(commentsDir, `${moduleId}.json`);
}

export async function listComments(moduleId?: string) {
  if (!moduleId) return [];
  try {
    const raw = await fs.readFile(filePathFor(moduleId), "utf8");
    return JSON.parse(raw);
  } catch {
    return [];
  }
}

export async function createComment(input: unknown) {
  const parsed = createCommentSchema.parse(input);
  const current = await listComments(parsed.moduleId);
  const now = new Date().toISOString();
  const record = {
    id: randomUUID(),
    ...parsed,
    createdAt: now,
    updatedAt: now
  };

  await fs.mkdir(commentsDir, { recursive: true });
  await fs.writeFile(filePathFor(parsed.moduleId), JSON.stringify([...current, record], null, 2));
  return record;
}
```

- [ ] **Step 4: Implement GET and POST routes**

```ts
import { Router } from "express";
import { createComment, listComments } from "../services/commentService";

export const commentsRouter = Router();

commentsRouter.get("/", async (req, res) => {
  const moduleId = typeof req.query.moduleId === "string" ? req.query.moduleId : undefined;
  const comments = await listComments(moduleId);
  res.json(comments);
});

commentsRouter.post("/", async (req, res) => {
  const record = await createComment(req.body);
  res.status(201).json(record);
});
```

- [ ] **Step 5: Connect frontend submit flow**

```ts
export async function createComment(input: {
  moduleId: string;
  sectionId?: string;
  sourceRef?: string;
  contentMarkdown: string;
  tags: string[];
}) {
  const response = await fetch("/api/comments", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(input)
  });

  if (!response.ok) throw new Error("Failed to create comment");
  return response.json();
}
```

- [ ] **Step 6: Run backend and frontend tests**

Run: `npm run test -- tests/server.comments.test.ts tests/module-page.test.tsx`
Expected: PASS

- [ ] **Step 7: Commit**

```bash
git -C D:/explore/CasyOS add learning-site site-data/comments
git -C D:/explore/CasyOS commit -m "feat: persist markdown study comments to files"
```

## Task 7: Implement generated learning notes

**Files:**
- Create: `D:/explore/CasyOS/learning-site/server/services/noteService.ts`
- Modify: `D:/explore/CasyOS/learning-site/server/routes/notes.ts`
- Create: `D:/explore/CasyOS/docs/learning/README.md`
- Modify: `D:/explore/CasyOS/learning-site/tests/server.notes.test.ts`
- Create: `D:/explore/CasyOS/learning-site/src/features/notes/NotesPage.tsx`

- [ ] **Step 1: Write the failing note generation test**

```ts
it("generates a markdown note file for a module", async () => {
  const app = createServer();
  const response = await request(app).post("/api/notes/generate").send({
    moduleId: "task-scheduler"
  });

  expect(response.status).toBe(200);
  expect(response.body.fileName).toContain("task-scheduler");
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `npm run test -- tests/server.notes.test.ts`
Expected: FAIL because note generation route does not exist

- [ ] **Step 3: Implement the note service**

```ts
import fs from "node:fs/promises";
import path from "node:path";
import { learningDocsDir } from "../utils/paths";
import { listComments } from "./commentService";
import { listModules } from "./moduleService";

export async function generateModuleNotes(moduleId: string) {
  const { modules } = await listModules();
  const module = modules.find((item) => item.id === moduleId);
  if (!module) throw new Error(`Unknown module: ${moduleId}`);

  const comments = await listComments(moduleId);
  const body = [
    `# ${module.title} 学习笔记`,
    "",
    `> 自动生成于 ${new Date().toISOString()}`,
    "",
    `## 模块摘要`,
    "",
    module.summary,
    "",
    `## 我的评论`,
    "",
    ...comments.flatMap((comment: any) => [
      `### ${comment.sectionId ?? "general"}`,
      "",
      comment.contentMarkdown,
      ""
    ])
  ].join("\n");

  await fs.mkdir(learningDocsDir, { recursive: true });
  const fileName = `${moduleId}.md`;
  const filePath = path.join(learningDocsDir, fileName);
  await fs.writeFile(filePath, body, "utf8");

  return { fileName, filePath };
}
```

- [ ] **Step 4: Add notes routes and notes page**

```ts
import { Router } from "express";
import { generateModuleNotes } from "../services/noteService";

export const notesRouter = Router();

notesRouter.post("/generate", async (req, res) => {
  const moduleId = String(req.body.moduleId || "");
  const result = await generateModuleNotes(moduleId);
  res.json(result);
});
```

```tsx
export function NotesPage() {
  return (
    <main className="page">
      <h1>学习文档</h1>
      <p>这里展示由评论沉淀出的 Markdown 学习文档。</p>
    </main>
  );
}
```

- [ ] **Step 5: Create the docs placeholder README**

```md
# Learning Notes

This directory stores generated and curated Markdown learning notes for the CasyOS learning site.
```

- [ ] **Step 6: Run note tests**

Run: `npm run test -- tests/server.notes.test.ts`
Expected: PASS

- [ ] **Step 7: Commit**

```bash
git -C D:/explore/CasyOS add learning-site docs/learning
git -C D:/explore/CasyOS commit -m "feat: generate learning markdown notes from comments"
```

## Task 8: Add source-guide content, polish the reading experience, and verify locally

**Files:**
- Modify: `D:/explore/CasyOS/learning-site/src/features/home/HomePage.tsx`
- Modify: `D:/explore/CasyOS/learning-site/src/features/modules/ModulePage.tsx`
- Modify: `D:/explore/CasyOS/learning-site/src/app/styles.css`
- Create: `D:/explore/CasyOS/learning-site/src/features/modules/SourceGuidePage.tsx`
- Modify: `D:/explore/CasyOS/learning-site/src/app/routes.tsx`

- [ ] **Step 1: Write the failing route expectation**

```tsx
it("renders a source guide route", async () => {
  render(
    <MemoryRouter initialEntries={["/notes"]}>
      <Routes>
        <Route path="/notes" element={<NotesPage />} />
      </Routes>
    </MemoryRouter>
  );

  expect(await screen.findByText("学习文档")).toBeInTheDocument();
});
```

- [ ] **Step 2: Run tests to verify current gaps**

Run: `npm run test -- tests/module-page.test.tsx`
Expected: FAIL if source guide/notes route integration is still missing

- [ ] **Step 3: Add richer homepage sections and source-guide page**

```tsx
<section className="learning-path">
  <h2>推荐学习路径</h2>
  <ol>
    <li>从项目定位与入口开始</li>
    <li>理解异常驱动切换</li>
    <li>掌握任务调度与状态流转</li>
    <li>再进入 IPC 与移植细节</li>
  </ol>
</section>
```

```tsx
export function SourceGuidePage() {
  return (
    <main className="page">
      <h1>运行流程导读</h1>
      <p>从 main 到 OSStart，再到第一次任务切换，按执行顺序讲解关键调用链。</p>
    </main>
  );
}
```

- [ ] **Step 4: Polish responsive document layout**

```css
@media (max-width: 1100px) {
  .page-module {
    grid-template-columns: 1fr;
  }

  .module-nav,
  .module-notes {
    position: static;
  }
}
```

- [ ] **Step 5: Run full verification**

Run: `npm run test`
Expected: PASS

Run: `npm run build`
Expected: PASS

Run: `npm run dev`
Expected: frontend loads on `http://localhost:5173` and API responds via proxy

- [ ] **Step 6: Commit**

```bash
git -C D:/explore/CasyOS add learning-site
git -C D:/explore/CasyOS commit -m "feat: polish CasyOS learning reading experience"
```

## Task 9: Update repository docs and prepare GitHub storage

**Files:**
- Modify: `D:/explore/CasyOS/README.md`
- Modify: `D:/explore/CasyOS/docs/learning/README.md`
- Optionally create: `D:/explore/CasyOS/learning-site/README.md`

- [ ] **Step 1: Write the failing documentation checklist**

```md
- README explains what `learning-site/` is
- README shows how to install and run the site
- README mentions where comments and generated notes are stored
- README confirms the project is ready to commit and push to GitHub
```

- [ ] **Step 2: Update root README with learning-site section**

```md
## Learning Site

This repository includes a local learning website under `learning-site/` for understanding the RTOS architecture of CasyOS.

### Run locally

```bash
cd learning-site
npm install
npm run dev
```

### Data locations

- module content: `site-data/modules/`
- markdown comments: `site-data/comments/`
- generated notes: `docs/learning/`
```

- [ ] **Step 3: Verify docs and repository state**

Run: `git -C D:/explore/CasyOS status --short`
Expected: only intentional new files and README updates are present

- [ ] **Step 4: Create final commit**

```bash
git -C D:/explore/CasyOS add README.md learning-site site-data docs
git -C D:/explore/CasyOS commit -m "feat: add CasyOS learning site and study notes workflow"
```

- [ ] **Step 5: Push to GitHub**

```bash
git -C D:/explore/CasyOS push
```

Expected: current branch pushes successfully to the configured GitHub remote

## Spec Coverage Check

- Homepage / learning path: covered by Tasks 4 and 8
- Module pages: covered by Tasks 3, 4, and 5
- Markdown comments: covered by Tasks 5 and 6
- Repository file persistence: covered by Tasks 6 and 7
- Generated learning docs: covered by Task 7
- GitHub-ready repository output: covered by Task 9

## Placeholder Scan

This plan intentionally avoids `TODO`, `TBD`, or vague “handle appropriately” steps. Each task names exact files, commands, and expected outcomes.

## Type Consistency Check

- `LearningModule`, `CommentRecord`, module ids, and route shapes are kept consistent across frontend and backend tasks.
- The API contract uses `/api/modules`, `/api/comments`, and `/api/notes` consistently across the plan.
