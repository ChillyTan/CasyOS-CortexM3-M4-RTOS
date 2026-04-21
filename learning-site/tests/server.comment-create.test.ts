// @vitest-environment node

import fs from "node:fs/promises";
import path from "node:path";
import request from "supertest";
import { afterEach, describe, expect, it } from "vitest";
import { createServer } from "../server/app";

const commentsFile = path.join(process.cwd(), "..", "site-data", "comments", "task-scheduler.json");

afterEach(async () => {
  await fs.rm(commentsFile, { force: true });
});

describe("comments api", () => {
  it("creates a markdown comment file for a module", async () => {
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
});
