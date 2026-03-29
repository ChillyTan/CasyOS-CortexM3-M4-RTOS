import path from "node:path";

export const repoRoot = path.resolve(process.cwd(), "..");
export const modulesDir = path.join(repoRoot, "site-data", "modules");
export const commentsDir = path.join(repoRoot, "site-data", "comments");
export const learningDocsDir = path.join(repoRoot, "docs", "learning");
