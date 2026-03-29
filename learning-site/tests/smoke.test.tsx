import { render, screen } from "@testing-library/react";
import { App } from "../src/app/App";

describe("app smoke", () => {
  it("renders the learning workspace placeholder", () => {
    render(<App />);

    expect(screen.getByText("学习网站骨架已启动")).toBeInTheDocument();
  });
});
