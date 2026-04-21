import { useState } from "react";
import { MarkdownEditor } from "../../components/MarkdownEditor";
import { MarkdownPreview } from "../../components/MarkdownPreview";
import { createComment } from "../../lib/api";

type CommentPanelProps = {
  moduleId: string;
  onCreated: () => Promise<void> | void;
};

export function CommentPanel({ moduleId, onCreated }: CommentPanelProps) {
  const [value, setValue] = useState("");
  const [saving, setSaving] = useState(false);

  async function handleSubmit() {
    if (!value.trim()) {
      return;
    }

    setSaving(true);
    await createComment({
      moduleId,
      contentMarkdown: value,
      tags: []
    });
    setValue("");
    await onCreated();
    setSaving(false);
  }

  return (
    <section className="comment-panel">
      <h2>学习批注</h2>
      <MarkdownEditor value={value} onChange={setValue} />
      <button className="primary-link comment-submit" disabled={saving} onClick={() => void handleSubmit()}>
        {saving ? "保存中..." : "保存评论"}
      </button>
      <MarkdownPreview value={value} />
    </section>
  );
}
