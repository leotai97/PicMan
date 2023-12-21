DELIMITER $$

CREATE OR REPLACE PROCEDURE pic_mgr.hashtag_folder_insert_proc
(
 iName     VARCHAR(255),
 iFolderID INT 
)

BEGIN

INSERT INTO hashtag_folder(name,folder_id) VALUES (iName, iFolderID);  
SELECT LAST_INSERT_ID();

END$$

delimiter ;