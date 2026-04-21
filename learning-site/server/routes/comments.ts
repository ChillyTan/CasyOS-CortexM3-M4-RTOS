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
