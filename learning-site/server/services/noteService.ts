import fs from "node:fs/promises";
import path from "node:path";
import { listComments } from "./commentService";
import { getModule } from "./moduleService";
import { learningDocsDir } from "../utils/paths";

function renderTags(tags: string[]) {
  if (!tags.length) {
    return "无";
  }

  return tags.map((tag) => `\`${tag}\``).join(" ");
}

export async function generateModuleNote(moduleId: string) {
  const module = await getModule(moduleId);

  if (!module) {
    return null;
  }

  const comments = await listComments(moduleId);
  const lines = [
    `# ${module.title}`,
    "",
    `> 模块 ID: \`${module.id}\``,
    "",
    "## 模块摘要",
    "",
    module.summary,
    "",
    "## 先修模块",
    "",
    module.prerequisites.length ? module.prerequisites.map((item) => `- ${item}`).join("\n") : "- 无",
    "",
    "## 标签",
    "",
    renderTags(module.tags),
    "",
    "## 关键源码",
    "",
    module.files.length ? module.files.map((item) => `- ${item}`).join("\n") : "- 无",
    "",
    "## 模块拆解",
    ""
  ];

  for (const section of module.sections) {
    lines.push(`### ${section.title}`, "", section.body, "");
  }

  lines.push("## 我的评论与批注", "");

  if (!comments.length) {
    lines.push("暂无评论。", "");
  } else {
    for (const comment of comments) {
      lines.push(`### ${comment.createdAt}`, "");

      if (comment.sectionId) {
        lines.push(`- 关联小节: \`${comment.sectionId}\``);
      }

      if (comment.sourceRef) {
        lines.push(`- 源码锚点: \`${comment.sourceRef}\``);
      }

      if (comment.tags.length) {
        lines.push(`- 标签: ${renderTags(comment.tags)}`);
      }

      if (comment.sectionId || comment.sourceRef || comment.tags.length) {
        lines.push("");
      }

      lines.push(comment.contentMarkdown, "");
    }
  }

  const fileName = `${module.id}.md`;
  const filePath = path.join(learningDocsDir, fileName);
  await fs.mkdir(learningDocsDir, { recursive: true });
  await fs.writeFile(filePath, lines.join("\n"), "utf8");

  return { fileName, filePath, module };
}
