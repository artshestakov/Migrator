--CREATE OR REPLACE DIRECTORY MIGRATOR AS 'G:\Shara\Migrator';
--GRANT READ ON DIRECTORY MIGRATOR TO PUBLIC; 
--SELECT * FROM all_directories;

SET SERVEROUTPUT ON
SET DEFINE OFF

DECLARE
    v_smd_file UTL_FILE.FILE_TYPE;
    v_primary_field VARCHAR2(4000) := NULL;
    v_table_comment VARCHAR2(4000) := NULL;
    v_field_default VARCHAR2(4000) := NULL;
    v_field_comment VARCHAR2(4000) := NULL;
    v_index_count NUMBER := NULL;
    v_foreign_count NUMBER := NULL;

BEGIN
    v_smd_file := UTL_FILE.FOPEN('MIGRATOR', 'transit.xml', 'w');
    UTL_FILE.PUT_LINE(v_smd_file, '<SMD>');
    UTL_FILE.PUT_LINE(v_smd_file, '');

    --Вытаскиваем таблицы
    FOR t IN (SELECT table_name FROM user_tables WHERE table_name NOT LIKE '%$%' ORDER BY table_name)
    LOOP

        --Вытаскиваем поля первичного ключа
        SELECT LISTAGG(cols.column_name, ';') WITHIN GROUP (ORDER BY cols.column_name)
        INTO v_primary_field
        FROM all_constraints cons, all_cons_columns cols
        WHERE cols.table_name = t.table_name
        AND cons.constraint_type = 'P'
        AND cons.constraint_name = cols.constraint_name
        AND cons.owner = cols.owner
        AND cons.owner = (SELECT USER FROM dual)
        ORDER BY cols.position;

        --Вытаскиваем комментарий для таблицы
        SELECT comments INTO v_table_comment
        FROM user_tab_comments
        WHERE table_name = t.table_name;

        UTL_FILE.PUT(v_smd_file, '    <Table Name="' || t.table_name || '"');

        --Если есть первичный ключ - добавим его в схему
        IF v_primary_field IS NOT NULL THEN
            UTL_FILE.PUT(v_smd_file, ' PrimaryField="' || v_primary_field || '"');
        END IF;

        --Если комментарий есть - добавляем его в схему
        IF v_table_comment IS NOT NULL THEN
            v_table_comment := REPLACE(v_table_comment, '"', '&quot;');
            UTL_FILE.PUT(v_smd_file, ' Comment="' || v_table_comment || '"');
        END IF;

        UTL_FILE.PUT(v_smd_file, '>');
        UTL_FILE.PUT_LINE(v_smd_file, '');

        --Обойдём поля таблицы
        UTL_FILE.PUT_LINE(v_smd_file, '        <Fields>');
        FOR f IN
        (
            SELECT c.column_name,
            CASE
                WHEN c.data_type = 'VARCHAR2' THEN 'VARCHAR' || '(' || c.data_length || ')' 
                WHEN c.data_type = 'CHAR' THEN 'CHAR' || '(' || c.data_length || ')'
                WHEN c.data_type = 'NUMBER' AND c.data_precision IS NOT NULL THEN 'NUMERIC' || '(' || c.data_precision || ')'
                WHEN c.data_type IN ('NUMBER', 'LONG') THEN 'NUMERIC'
                WHEN c.data_type IN ('RAW', 'BFILE', 'CLOB', 'ANYDATA', 'ROWID') THEN 'VARCHAR'
                WHEN c.data_type LIKE '%TIMESTAMP%' THEN 'TIMESTAMP WITHOUT TIME ZONE'
                ELSE c.data_type
            END AS data_type,
            c.data_default,
            c.nullable,
            cm.comments
            FROM user_tab_columns c
            LEFT JOIN user_col_comments cm ON cm.table_name = c.table_name AND cm.column_name = c.column_name
            WHERE c.table_name = t.table_name
            ORDER BY c.column_id
        )
        LOOP
            UTL_FILE.PUT(v_smd_file, '            <Field Name="' || f.column_name || '"' || ' Type="' || f.data_type || '"');

            --Есть у поля есть значение по умолчанию - добавим его в схему
            IF f.data_default IS NOT NULL THEN
                v_field_default := f.data_default;

                --Убираем идиотский пробел в конце значения по умолчанию
                WHILE SUBSTR(v_field_default, -1) = ' '
                LOOP
                    v_field_default := SUBSTR(v_field_default, 1, LENGTH(v_field_default) - 1);
                END LOOP;

                --А так же, убираем идиотский перенос строки
                v_field_default := REPLACE(REPLACE(v_field_default, chr(13), ''), chr (10), '');

                --Эквиваленты
                IF upper(v_field_default) LIKE '%SYSDATE%' THEN
                    v_field_default := 'CURRENT_DATE';
                ELSIF upper(v_field_default) = 'SYSTIMESTAMP' THEN
                    v_field_default := 'CURRENT_TIMESTAMP';
                ELSIF upper(v_field_default) = 'SYS_GUID()' THEN
                    v_field_default := 'uuid_generate_v4()';
                ELSIF upper(v_field_default) LIKE '%USERENV%' THEN
                    v_field_default := '';
                ELSIF upper(v_field_default) = 'NULL' THEN
                    v_field_default := '';
                END IF;
                UTL_FILE.PUT(v_smd_file, ' DefaultValue="' || v_field_default || '"');
            END IF;

            --Если поле не должно быть пустым - учтём
            IF f.nullable = 'N' THEN
                UTL_FILE.PUT(v_smd_file, ' NotNull="true"');
            END IF;

            --Если комментарий есть - добавим в схему
            IF f.comments IS NOT NULL THEN
                v_field_comment := REPLACE(f.comments, '"', '&quot;');
                UTL_FILE.PUT(v_smd_file, ' Comment="' || v_field_comment || '"');
            END IF;

            UTL_FILE.PUT(v_smd_file, '/>');           
            UTL_FILE.PUT_LINE(v_smd_file, '');
        END LOOP;
        UTL_FILE.PUT_LINE(v_smd_file, '        </Fields>');

        --Проверим, есть ли индексы у таблицы
        SELECT COUNT(*) INTO v_index_count
        FROM user_indexes
        WHERE table_name = t.table_name
        AND index_name NOT IN (SELECT index_name FROM user_constraints WHERE table_name = t.table_name AND constraint_type = 'P')
        AND index_name NOT IN (SELECT constraint_name FROM user_constraints WHERE table_name = t.table_name)
        AND index_type = 'NORMAL';

        --Если индексы есть - добавляем секцию
        IF v_index_count > 0 THEN
            UTL_FILE.PUT_LINE(v_smd_file, '        <Indexes>');

            FOR idx IN
            (
                SELECT i.index_name, 
                (
                    SELECT LISTAGG(c.column_name, ';') WITHIN GROUP (ORDER BY c.column_name)
                    FROM user_ind_columns c
                    WHERE c.index_name = i.index_name
                ) AS fields,
                CASE WHEN i.uniqueness = 'UNIQUE' THEN 1 ELSE 0 END AS is_unique
                FROM user_indexes i
                WHERE i.table_name = t.table_name
                AND i.index_name NOT IN (SELECT index_name FROM user_constraints WHERE table_name = t.table_name AND constraint_type = 'P')
                AND i.index_name NOT IN (SELECT constraint_name FROM user_constraints WHERE table_name = t.table_name)
                AND index_type = 'NORMAL'
            )
            LOOP
                UTL_FILE.PUT(v_smd_file, '            <Index Name="' || idx.index_name || '" Fields="' || idx.fields || '"');

                IF idx.is_unique = 1 THEN
                    UTL_FILE.PUT(v_smd_file, ' Unique="true"');
                END IF;

                UTL_FILE.PUT_LINE(v_smd_file, '/>');
            END LOOP;

            UTL_FILE.PUT_LINE(v_smd_file, '        </Indexes>');
        END IF;

        --Проверим, есть ли внешние ключи у таблицы
        SELECT COUNT(*) INTO v_foreign_count
        FROM user_constraints
        WHERE constraint_type = 'R'
        AND table_name = t.table_name;

        --Если внешние ключи есть - добавим их в схему
        IF v_foreign_count > 0 THEN
            UTL_FILE.PUT_LINE(v_smd_file, '        <Foreigns>');

            FOR f IN
            (
                SELECT c.constraint_name AS foreign_name, cols.column_name AS field_name, cons_r.table_name AS reference_table, cols_r.column_name AS reference_field
                FROM user_constraints c
                LEFT JOIN user_cons_columns cols ON cols.constraint_name = c.constraint_name
                LEFT JOIN user_constraints cons_r ON cons_r.constraint_name = c.r_constraint_name
                LEFT JOIN user_cons_columns cols_r ON cols_r.constraint_name = c.r_constraint_name
                WHERE c.CONSTRAINT_TYPE = 'R'
                AND c.table_name = t.table_name
            )
            LOOP
                UTL_FILE.PUT(v_smd_file, '            <Foreign Name="' || f.foreign_name || '" Field="' || f.field_name || '" ReferenceTable="' || f.reference_table || '" ReferenceField="' || f.reference_field || '"/>');
                UTL_FILE.PUT_LINE(v_smd_file, '');
            END LOOP;

            UTL_FILE.PUT_LINE(v_smd_file, '        </Foreigns>');
        END IF;

        UTL_FILE.PUT_LINE(v_smd_file, '    </Table>');
        UTL_FILE.PUT_LINE(v_smd_file, '');
    END LOOP;

    UTL_FILE.PUT_LINE(v_smd_file, '</SMD>');
    UTL_FILE.FCLOSE(v_smd_file);
END;
