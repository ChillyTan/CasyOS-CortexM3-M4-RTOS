import { createServer } from "./app";

const app = createServer();
const port = 4173;

app.listen(port, () => {
  console.log(`CasyOS learning server listening on ${port}`);
});
