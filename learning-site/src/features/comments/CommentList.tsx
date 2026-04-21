import { MarkdownPreview } from "../../components/MarkdownPreview";
import type { CommentRecord } from "../../lib/types";

export function CommentList({ comments }: { comments: CommentRecord[] }) {
  if (comments.length === 0) {
    return <p className="comment-empty">这里还没有评论，试着写下第一条理解。</p>;
  }

  return (
    <div className="comment-list">
      {comments.map((comment) => (
        <article className="comment-card" key={comment.id}>
          <div className="comment-meta">
            <strong>{comment.sectionId ?? "general"}</strong>
            <span>{new Date(comment.createdAt).toLocaleString("zh-CN")}</span>
          </div>
          <MarkdownPreview value={comment.contentMarkdown} />
        </article>
      ))}
    </div>
  );
}
