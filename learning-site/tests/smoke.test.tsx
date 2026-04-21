import { render, screen } from "@testing-library/react";
import { App } from "../src/app/App";

describe("app smoke", () => {
  it("renders the learning workspace home page", () => {
    render(<App />);

    expect(screen.getByText("从源码到机制，系统理解 CasyOS")).toBeInTheDocument();
  });
});
