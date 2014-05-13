/* contrib/sprocket/sprocket--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION sprocket" to load this file. \quit

CREATE FUNCTION sprocket_change() RETURNS trigger
     AS 'MODULE_PATHNAME', 'sprocket_change'
     LANGUAGE C STRICT;

CREATE FUNCTION sprocket_ddl() RETURNS event_trigger
    AS 'MODULE_PATHNAME', 'sprocket_ddl' LANGUAGE C STRICT;