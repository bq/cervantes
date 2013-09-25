CREATE TABLE book(
    path TEXT primary key,
    isbn TEXT,
    lastTimeRead TEXT,
    readingState TEXT,
    openingState TEXT,
    title TEXT,
    author TEXT,
    thumbnail TEXT,
    pageCount INTEGER,
    lastReadLink TEXT,
    lastReadPage INTEGER,
    fontSize INTEGER,
    syncDate TEXT
);

CREATE TABLE location(
    id INTEGER PRIMARY KEY,
    type TEXT,
    text TEXT,
    note TEXT,
    url TEXT,
    bookmark TEXT,
    pos REAL,
    page INTEGER,
    font TEXT,
    fontSize REAL,
    tiltState INTEGER,
    book TEXT,
    FOREIGN KEY(book) REFERENCES book(path) ON DELETE CASCADE
);
