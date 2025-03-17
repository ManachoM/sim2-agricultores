-- Add proc_id column to aggregated_product_results if it doesn't exist
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT FROM information_schema.columns 
        WHERE table_name = 'aggregated_product_results' AND column_name = 'proc_id'
    ) THEN
        ALTER TABLE aggregated_product_results ADD COLUMN proc_id INTEGER DEFAULT 0;
        COMMENT ON COLUMN aggregated_product_results.proc_id IS 'The processor ID that generated this data point';
    END IF;
END $$;