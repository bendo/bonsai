{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE ScopedTypeVariables #-}

module App
    ( app
    ) where

import Database.SQLite.Simple
import qualified Data.Text as T


data Bonsai = Bonsai {
    timestamp :: T.Text,
    temperature :: Float,
    humidity :: Float,
    vbat :: Float,
    solar :: Float,
    leveltop :: Integer,
    levelbottom :: Integer,
    light :: Integer
} deriving Show

instance FromRow Bonsai where
    fromRow = Bonsai <$> field <*> field <*> field <*> field <*> field <*> field <*> field <*> field

app :: IO ()
app = do
    conn <- open $ "./data/bonsai.db"
    bonsai <- query conn "SELECT * FROM bonsai" () :: IO [Bonsai]
    let temp = map temperature bonsai
    let hum = map humidity bonsai
    let bat = map vbat bonsai
    let sol = map solar bonsai
    let lvltop = map leveltop bonsai
    let lvlbottom = map levelbottom bonsai
    let lg = map light bonsai
    putStrLn "Bonsai:"
    printMinMax "Temp" temp
    printMinMax "Humidity" hum
    printMinMax "Battery" bat
    printMinMax "Solar" sol
    printMinMax "Light" lg
    putStrLn ""


printMinMax :: (Show a, Foldable t, Ord a) => [Char] -> t a -> IO ()
printMinMax label xs = do
    putStr $ "Max " ++ label ++ ": "
    print $ maximum xs
    putStr $ "Min " ++ label ++ ": "
    print $ minimum xs
