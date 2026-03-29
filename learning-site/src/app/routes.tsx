import { createBrowserRouter } from "react-router-dom";

function PlaceholderPage() {
  return (
    <main className="page">
      <p className="eyebrow">CasyOS Learning Workspace</p>
      <h1>学习网站骨架已启动</h1>
      <p>下一步会接入模块内容、评论区和学习文档。</p>
    </main>
  );
}

export const router = createBrowserRouter([
  {
    path: "/",
    element: <PlaceholderPage />
  }
]);
