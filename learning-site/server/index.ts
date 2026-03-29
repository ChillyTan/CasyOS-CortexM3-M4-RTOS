import express from "express";

const app = express();
const port = 4173;

app.get("/api/health", (_req, res) => {
  res.json({ ok: true });
});

app.listen(port, () => {
  console.log(`CasyOS learning server listening on ${port}`);
});
