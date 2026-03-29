import { Router } from "express";
import { listModules } from "../services/moduleService";

export const modulesRouter = Router();

modulesRouter.get("/", async (_req, res) => {
  const data = await listModules();
  res.json(data);
});

modulesRouter.get("/:moduleId", async (req, res) => {
  const data = await listModules();
  const module = data.modules.find((item) => item.id === req.params.moduleId);

  if (!module) {
    res.status(404).json({ message: "Module not found" });
    return;
  }

  res.json(module);
});
