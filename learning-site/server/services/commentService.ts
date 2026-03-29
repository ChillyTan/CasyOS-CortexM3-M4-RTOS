import fs from "node:fs/promises";
import path from "node:path";
import { randomUUID } from "node:crypto";
import { commentsDir } from "../utils/paths";

export type CommentRecord = {
  id: string;
  moduleId: string;
  sectionId?: string;
  sourceRef?: string;
  contentMarkdown: string;
  tags: string[];
  createdAt: string;
  updatedAt: string;
};

type CreateCommentInput = {
  moduleId: string;
  sectionId?: string;
  sourceRef?: string;
  contentMarkdown: string;
  tags?: string[];
};

function getCommentsFile(moduleId: string) {
  return path.join(commentsDir, `${moduleId}.json`);
}

export async function listComments(moduleId?: string): Promise<CommentRecord[]> {
  if (!moduleId) {
    return [];
  }

  try {
    const raw = await fs.readFile(getCommentsFile(moduleId), "utf8");
    return JSON.parse(raw) as CommentRecord[];
  } catch {
    return [];
  }
}

export async function createComment(input: CreateCommentInput): Promise<CommentRecord> {
  const now = new Date().toISOString();
  const record: CommentRecord = {
    id: randomUUID(),
    moduleId: input.moduleId,
    sectionId: input.sectionId,
    sourceRef: input.sourceRef,
    contentMarkdown: input.contentMarkdown,
    tags: input.tags ?? [],
    createdAt: now,
    updatedAt: now
  };

  const current = await listComments(input.moduleId);

  await fs.mkdir(commentsDir, { recursive: true });
  await fs.writeFile(getCommentsFile(input.moduleId), JSON.stringify([...current, record], null, 2), "utf8");

  return record;
}
