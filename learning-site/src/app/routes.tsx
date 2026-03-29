import { createBrowserRouter } from "react-router-dom";
import { Layout } from "../components/Layout";
import { HomePage } from "../features/home/HomePage";
import { ModulePage } from "../features/modules/ModulePage";
import { NotesPage } from "../features/notes/NotesPage";

export const router = createBrowserRouter([
  {
    path: "/",
    element: <Layout />,
    children: [
      {
        index: true,
        element: <HomePage />
      },
      {
        path: "modules/:moduleId",
        element: <ModulePage />
      },
      {
        path: "notes",
        element: <NotesPage />
      }
    ]
  }
]);
