import type { CommentRecord, GeneratedNote, LearningModule, ModuleIndexResponse } from "./types";

export async function fetchModules(): Promise<ModuleIndexResponse> {
  const response = await fetch("/api/modules");
  if (!response.ok) {
    throw new Error("Failed to load modules");
  }

  return response.json();
}

export async function fetchModuleById(moduleId: string): Promise<LearningModule> {
  const response = await fetch(`/api/modules/${moduleId}`);
  if (!response.ok) {
    throw new Error("Failed to load module");
  }

  return response.json();
}

export async function fetchComments(moduleId: string): Promise<CommentRecord[]> {
  const response = await fetch(`/api/comments?moduleId=${moduleId}`);
  if (!response.ok) {
    throw new Error("Failed to load comments");
  }

  return response.json();
}

export async function createComment(input: {
  moduleId: string;
  sectionId?: string;
  sourceRef?: string;
  contentMarkdown: string;
  tags: string[];
}) {
  const response = await fetch("/api/comments", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(input)
  });

  if (!response.ok) {
    throw new Error("Failed to create comment");
  }

  return response.json();
}

export async function generateNote(moduleId: string): Promise<{ fileName: string; filePath: string; moduleId: string }> {
  const response = await fetch("/api/notes/generate", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({ moduleId })
  });

  if (!response.ok) {
    throw new Error("Failed to generate note");
  }

  return response.json();
}

export async function fetchNotes(): Promise<{ notes: GeneratedNote[] }> {
  const response = await fetch("/api/notes");
  if (!response.ok) {
    throw new Error("Failed to load notes");
  }

  return response.json();
}
