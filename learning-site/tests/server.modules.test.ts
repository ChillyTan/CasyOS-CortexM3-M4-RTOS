// @vitest-environment node

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
