import { RouterProvider } from "react-router-dom";
import { router } from "./routes";
import "./styles.css";

export function App() {
  return <RouterProvider router={router} />;
}
