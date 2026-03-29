// @vitest-environment node

import fs from "node:fs/promises";
import path from "node:path";
import request from "supertest";
import { afterEach, beforeEach, describe, expect, it } from "vitest";
import { createServer } from "../server/app";

const commentsFile = path.join(process.cwd(), "..", "site-data", "comments", "task-scheduler.json");
const notesFile = path.join(process.cwd(), "..", "docs", "learning", "task-scheduler.md");

beforeEach(async () => {
  await fs.mkdir(path.dirname(commentsFile), { recursive: true });
  await fs.writeFile(
    commentsFile,
    JSON.stringify([
      {
        id: "1",
        moduleId: "task-scheduler",
        contentMarkdown: "## 调度笔记\n\n这里记录我的调度理解。",
        tags: [],
        createdAt: "2026-03-29T00:00:00.000Z",
        updatedAt: "2026-03-29T00:00:00.000Z"
      }
    ]),
    "utf8"
  );
});

afterEach(async () => {
  await fs.rm(commentsFile, { force: true });
  await fs.rm(notesFile, { force: true });
});

describe("notes api", () => {
  it("generates a markdown note file for a module", async () => {
    const app = createServer();
    const response = await request(app).post("/api/notes/generate").send({
      moduleId: "task-scheduler"
    });

    expect(response.status).toBe(200);
    expect(response.body.fileName).toContain("task-scheduler");
  });
});
