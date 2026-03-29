import { render, screen } from "@testing-library/react";
import { MemoryRouter, Route, Routes } from "react-router-dom";
import { describe, expect, it, vi } from "vitest";
import { ModulePage } from "../src/features/modules/ModulePage";

vi.mock("../src/lib/api", () => ({
  fetchModuleById: async () => ({
    id: "task-scheduler",
    title: "任务管理与调度",
    summary: "summary",
    category: "kernel-core",
    prerequisites: [],
    files: [],
    tags: [],
    sections: []
  }),
  fetchComments: async () => []
}));

describe("module page", () => {
  it("renders the module title", async () => {
    render(
      <MemoryRouter initialEntries={["/modules/task-scheduler"]}>
        <Routes>
          <Route path="/modules/:moduleId" element={<ModulePage />} />
        </Routes>
      </MemoryRouter>
    );

    expect(await screen.findByRole("heading", { name: "任务管理与调度", level: 1 })).toBeInTheDocument();
  });
});
