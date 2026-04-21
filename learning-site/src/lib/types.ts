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

export type ModuleIndexResponse = {
  learningPath?: string[];
  modules: LearningModule[];
};

export type CommentRecord = {
  id: string;
  moduleId: string;
  sectionId?: string;
  sourceRef?: string;
  contentMarkdown: string;
  tags: string[];
  createdAt: string;
  updatedAt: string;
};

export type GeneratedNote = {
  moduleId: string;
  title: string;
  fileName: string;
  filePath: string;
  updatedAt: string;
};
