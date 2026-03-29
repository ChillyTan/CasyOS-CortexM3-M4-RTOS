import { useEffect, useState } from "react";
import { Link, useParams } from "react-router-dom";
import { fetchComments, fetchModuleById, generateNote } from "../../lib/api";
import type { CommentRecord, LearningModule } from "../../lib/types";
import { CommentList } from "../comments/CommentList";
import { CommentPanel } from "../comments/CommentPanel";

export function ModulePage() {
  const { moduleId = "" } = useParams();
  const [module, setModule] = useState<LearningModule | null>(null);
  const [comments, setComments] = useState<CommentRecord[]>([]);
  const [generating, setGenerating] = useState(false);
  const [notePath, setNotePath] = useState("");

  useEffect(() => {
    void fetchModuleById(moduleId).then(setModule);
    void fetchComments(moduleId).then(setComments);
  }, [moduleId]);

  async function handleGenerateNote() {
    if (!module) {
      return;
    }

    setGenerating(true);

    try {
      const result = await generateNote(module.id);
      setNotePath(result.filePath);
    } finally {
      setGenerating(false);
    }
  }

  if (!module) {
    return <main className="page page-module">Loading...</main>;
  }

  return (
    <main className="page page-module">
      <aside className="module-rail">
        <p className="eyebrow">Module</p>
        <h2>{module.title}</h2>
        <p>{module.summary}</p>
        <Link className="secondary-link" to="/">
          返回学习总览
        </Link>
      </aside>

      <section className="module-body">
        <h1>{module.title}</h1>
        <p>{module.summary}</p>
        {module.sections.map((section) => (
          <article className="module-section" key={section.id}>
            <h2>{section.title}</h2>
            <p>{section.body}</p>
          </article>
        ))}
      </section>

      <aside className="module-comments">
        <section className="comment-panel">
          <h2>学习文档</h2>
          <p>把模块讲解和你写下的 Markdown 评论汇总成正式学习文档。</p>
          <button className="secondary-link comment-submit" disabled={generating} onClick={() => void handleGenerateNote()}>
            {generating ? "生成中..." : "生成学习文档"}
          </button>
          {notePath ? <p className="note-path">已生成到：{notePath}</p> : null}
        </section>
        <CommentPanel moduleId={module.id} onCreated={async () => setComments(await fetchComments(module.id))} />
        <CommentList comments={comments} />
      </aside>
    </main>
  );
}
