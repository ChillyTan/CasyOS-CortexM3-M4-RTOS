// @vitest-environment node

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
