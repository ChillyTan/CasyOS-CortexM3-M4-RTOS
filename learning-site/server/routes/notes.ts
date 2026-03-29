import { Router } from "express";
import { generateModuleNote } from "../services/noteService";

export const notesRouter = Router();

notesRouter.get("/", (_req, res) => {
  res.json({ notes: [] });
});

notesRouter.post("/generate", async (req, res) => {
  const moduleId = typeof req.body?.moduleId === "string" ? req.body.moduleId : "";

  if (!moduleId) {
    res.status(400).json({ message: "moduleId is required" });
    return;
  }

  const result = await generateModuleNote(moduleId);

  if (!result) {
    res.status(404).json({ message: "Module not found" });
    return;
  }

  res.json({
    moduleId,
    fileName: result.fileName,
    filePath: result.filePath
  });
});
