import { useEffect, useState } from "react";
import { Link } from "react-router-dom";
import { fetchNotes } from "../../lib/api";
import type { GeneratedNote } from "../../lib/types";

export function NotesPage() {
  const [notes, setNotes] = useState<GeneratedNote[]>([]);

  useEffect(() => {
    void fetchNotes().then((response) => setNotes(response.notes));
  }, []);

  return (
    <main className="page">
      <p className="eyebrow">Notes</p>
      <h1>学习文档</h1>
      <p>这里汇总由模块讲解和你的 Markdown 评论生成的学习文档，方便持续复盘和整理。</p>
      <section className="notes-list">
        {notes.length ? (
          notes.map((note) => (
            <article className="comment-card" key={note.filePath}>
              <p className="eyebrow">Module Note</p>
              <h2>{note.title}</h2>
              <p>{note.fileName}</p>
              <p className="note-path">{note.filePath}</p>
              <p>最近更新：{new Date(note.updatedAt).toLocaleString("zh-CN")}</p>
              <Link className="secondary-link" to={`/modules/${note.moduleId}`}>
                回到模块页
              </Link>
            </article>
          ))
        ) : (
          <article className="comment-card">
            <h2>还没有生成学习文档</h2>
            <p>先进入任意模块页，写下你的评论，再点击“生成学习文档”。</p>
          </article>
        )}
      </section>
    </main>
  );
}
