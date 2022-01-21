package com.taosdata.jdbc.ws;

import com.taosdata.jdbc.AbstractConnection;
import com.taosdata.jdbc.TSDBDriver;
import com.taosdata.jdbc.TSDBError;
import com.taosdata.jdbc.TSDBErrorNumbers;
import com.taosdata.jdbc.rs.RestfulDatabaseMetaData;

import java.sql.DatabaseMetaData;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;

public class WSConnection extends AbstractConnection {
    private final Transport transport;
    private final DatabaseMetaData metaData;
    private final String database;
    private boolean fetchType;

    public WSConnection(String url, Properties properties, Transport transport, String database, boolean fetchType) {
        super(properties);
        this.transport = transport;
        this.database = database;
        this.fetchType = fetchType;
        this.metaData = new RestfulDatabaseMetaData(url, properties.getProperty(TSDBDriver.PROPERTY_KEY_USER), this);
    }

    @Override
    public Statement createStatement() throws SQLException {
        if (isClosed())
            throw TSDBError.createSQLException(TSDBErrorNumbers.ERROR_CONNECTION_CLOSED);

//        return new WSStatement(transport, database , fetchType);
        return null;
    }

    @Override
    public PreparedStatement prepareStatement(String sql) throws SQLException {
        if (isClosed())
            throw TSDBError.createSQLException(TSDBErrorNumbers.ERROR_CONNECTION_CLOSED);

//        return new WSPreparedStatement();
        return null;
    }

    @Override
    public void close() throws SQLException {
        transport.close();
    }

    @Override
    public boolean isClosed() throws SQLException {
        return transport.isClosed();
    }

    @Override
    public DatabaseMetaData getMetaData() throws SQLException {
        if (isClosed()) {
            throw TSDBError.createSQLException(TSDBErrorNumbers.ERROR_CONNECTION_CLOSED);
        }
        return this.metaData;
    }
}
