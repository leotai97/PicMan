DELIMITER $$

CREATE OR REPLACE PROCEDURE pic_mgr.picture_insert_proc
(
 iFldr     INT,
 iName     VARCHAR(1024),
 iWidth    INT,
 iHeight   INT
)

BEGIN

INSERT INTO picture(folder_id, file_name, picture_added, width, height) VALUES (iFldr,iName, NOW(), iWidth, iHeight); 
SELECT LAST_INSERT_ID();

END$$

delimiter ;