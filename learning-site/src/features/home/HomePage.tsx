import { Link } from "react-router-dom";

export function HomePage() {
  return (
    <main className="page page-home">
      <section className="hero">
        <p className="eyebrow">CasyOS Learning Workspace</p>
        <h1>从源码到机制，系统理解 CasyOS</h1>
        <p>
          这个站点不是普通代码浏览器，而是一个面向学习的讲解工作台。
          你可以按推荐路径理解调度、异常、IPC 与移植过程，并把你的评论沉淀成学习文档。
        </p>
        <div className="hero-actions">
          <Link className="primary-link" to="/modules/project-overview">
            从项目总览开始
          </Link>
          <Link className="secondary-link" to="/modules/task-scheduler">
            直接进入调度模块
          </Link>
        </div>
      </section>

      <section className="learning-path">
        <h2>推荐学习路径</h2>
        <ol>
          <li>项目总览：先明确 CasyOS 的定位和边界</li>
          <li>启动与异常：掌握 SVC、PendSV、SysTick 的配合</li>
          <li>任务与调度：理解 TCB、就绪队列和优先级位图</li>
          <li>IPC：依次看信号量、互斥量、消息队列和事件标志组</li>
          <li>移植与 Demo：回到具体工程看如何落地</li>
        </ol>
      </section>
    </main>
  );
}
