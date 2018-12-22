BEGIN TRANSACTION;

CREATE TABLE client_tbl (
    id INTEGER PRIMARY KEY ASC,
    acronym TEXT,
    name TEXT,
    default_rate INTEGER,
    default_charge_quantum INTEGER
);

CREATE TABLE project_tbl (
    id INTEGER PRIMARY KEY ASC,
    client_id INTEGER REFERENCES client_tbl,
    project_id INTEGER REFERENCE project_tbl DEFAULT NULL,
    title TEXT,
    rate INTEGER,
    charge_quantum INTEGER
);

CREATE TABLE eventtype_enum_tbl (
    enum_val INTEGER PRIMARY KEY ASC,
    name TEXT NOT NULL
);

INSERT INTO eventtype_enum_tbl(enum_val, name) VALUES (0, 'Start');
INSERT INTO eventtype_enum_tbl(enum_val, name) VALUES (1, 'Stop');
INSERT INTO eventtype_enum_tbl(enum_val, name) VALUES (2, 'Pause');

CREATE TABLE event_tbl(
    id INTEGER PRIMARY KEY ASC,
    project_id INTEGER REFERENCES project_tbl,
    when_ts INTEGER,
    type_enum INTEGER /* see eventtype_enum_tbl */
);

CREATE INDEX event_idx_0 ON event_tbl (when_ts);
CREATE INDEX event_idx_1 ON event_tbl (project_id);

CREATE TABLE tablename_enum_tbl (
    enum_val INTEGER PRIMARY KEY ASC,
    name TEXT NOT NULL
);

INSERT INTO tablename_enum_tbl(enum_val, name) VALUES (1, 'client_tbl');
INSERT INTO tablename_enum_tbl(enum_val, name) VALUES (2, 'project_tbl');

CREATE TABLE notes_tbl(
    id INTEGER PRIMARY KEY ASC,
    parent_tbl_enum INTEGER,
    parent_id INTEGER,
    when_ts INTEGER,
    text TEXT
);

CREATE TRIGGER client_delete_trg
  BEFORE DELETE ON client_tbl
    FOR EACH ROW
    BEGIN
      DELETE FROM project_tbl WHERE project_tbl.client_id = OLD.id;
      DELETE FROM notes_tbl WHERE notes_tbl.parent_tbl_enum = 1 AND notes_tbl.parent_id = OLD.id;
    END;

CREATE TRIGGER project_delete_trg
  BEFORE DELETE ON project_tbl
    FOR EACH ROW
    BEGIN
      DELETE FROM event_tbl WHERE event_tbl.project_id = OLD.id;
      DELETE FROM notes_tbl WHERE notes_tbl.parent_tbl_enum = 2 AND notes_tbl.parent_id = OLD.id;
      DELETE FROM project_tbl WHERE project_tbl.project_id = OLD.id;
    END;

CREATE TRIGGER event_project_trg
  BEFORE INSERT ON event_tbl
    FOR EACH ROW WHEN (SELECT COUNT(*) FROM project_tbl WHERE project_tbl.id = NEW.project_id) = 0
    BEGIN
      SELECT RAISE(ROLLBACK, 'foreign-key violation: project_tbl.id');
    END;

CREATE TRIGGER project_client_trg
  BEFORE INSERT ON project_tbl
    FOR EACH ROW WHEN (SELECT COUNT(*) FROM client_tbl WHERE client_tbl.id = NEW.client_id) = 0
    BEGIN
      SELECT RAISE(ROLLBACK, 'foreign-key violation: client_tbl.id');
    END;

COMMIT;
