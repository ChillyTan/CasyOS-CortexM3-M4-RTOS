type MarkdownEditorProps = {
  value: string;
  onChange: (nextValue: string) => void;
};

export function MarkdownEditor({ value, onChange }: MarkdownEditorProps) {
  return (
    <textarea
      className="markdown-editor"
      value={value}
      placeholder="写下你的理解、疑问或批注..."
      onChange={(event) => onChange(event.target.value)}
    />
  );
}
