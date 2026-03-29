import express from "express";
import { commentsRouter } from "./routes/comments";
import { modulesRouter } from "./routes/modules";
import { notesRouter } from "./routes/notes";

export function createServer() {
  const app = express();

  app.use(express.json());
  app.use("/api/modules", modulesRouter);
  app.use("/api/comments", commentsRouter);
  app.use("/api/notes", notesRouter);

  return app;
}
