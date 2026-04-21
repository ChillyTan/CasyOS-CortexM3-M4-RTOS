import { Link, Outlet } from "react-router-dom";

export function Layout() {
  return (
    <div className="shell">
      <header className="site-header">
        <Link className="brand" to="/">
          CasyOS Learning Workspace
        </Link>
        <nav className="site-nav">
          <Link to="/">总览</Link>
          <Link to="/notes">学习文档</Link>
        </nav>
      </header>
      <Outlet />
    </div>
  );
}
