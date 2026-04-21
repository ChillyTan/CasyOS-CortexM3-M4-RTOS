export type LearningModule = {
  id: string;
  title: string;
  summary: string;
  category: string;
  prerequisites: string[];
  sections: Array<{
    id: string;
    title: string;
    body: string;
  }>;
  files: string[];
  tags: string[];
};
