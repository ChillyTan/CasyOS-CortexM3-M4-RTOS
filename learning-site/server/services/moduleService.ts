import fs from "node:fs/promises";
import path from "node:path";
import { modulesDir } from "../utils/paths";
import type { LearningModule } from "../utils/schemas";

type ModuleIndex = {
  learningPath: string[];
};

export async function listModules(): Promise<{ learningPath: string[]; modules: LearningModule[] }> {
  const indexPath = path.join(modulesDir, "index.json");
  const indexRaw = await fs.readFile(indexPath, "utf8");
  const index = JSON.parse(indexRaw) as ModuleIndex;

  const modules = await Promise.all(
    index.learningPath.map(async (id) => {
      const raw = await fs.readFile(path.join(modulesDir, `${id}.json`), "utf8");
      return JSON.parse(raw) as LearningModule;
    })
  );

  return { learningPath: index.learningPath, modules };
}

export async function getModule(moduleId: string): Promise<LearningModule | null> {
  const data = await listModules();
  return data.modules.find((item) => item.id === moduleId) ?? null;
}
